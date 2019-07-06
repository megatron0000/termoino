const connector = require('bau-arduino-data-logging').connector
const Event = require('bau-arduino-data-logging').Event
const debounce = require('debounce')
const LogRecord = require('./database').LogRecord
const bookshelf = require('./database').bookshelf
const moment = require('moment')
const mod = angular.module('ArduinoDataLogging', [])

/**
   * 
   * @returns {Promise<Array<any>>}
   */
function fetch(build) {
    return build(bookshelf.knex('logrecords'))
        .then(records => {
            records.forEach(record => record.timestamp = moment(new Date(record.timestamp)))
            return records
        })
}

mod.factory('events', ['$rootScope', function ($rootScope) {
    return {
        SERIALPORT_UPDATE: 'serialport update',
        DATA_IMPORTED: 'data imported',
        broadcast(event) {
            $rootScope.$broadcast(event)
        },
        on(eventname, callback) {
            $rootScope.$on(eventname, callback)
        }
    }
}])


mod.factory('serialport', ['events', function (events) {
    return {
        port: { isOpen: false },
        parser: null,
        is_connected: false,
        publish() {
            events.broadcast(events.SERIALPORT_UPDATE)
        }
    }
}])

mod.controller('DataImportController', ['$scope', 'serialport', 'events', function ($scope, serialport, events) {
    events.on(events.SERIALPORT_UPDATE, () => {
        reset_scope()
        $scope.is_connected = serialport.is_connected
        $scope.$apply()
        if ($scope.is_connected) {
            serialport.parser.command(Event.RECORD_COUNT_REQUEST, (count) => {
                $scope.new_record_count = count
                $scope.$apply()
            })
        }
    })

    function reset_scope() {
        $scope.is_connected = false
        $scope.new_record_count = 0
        $scope.progress = 0
        $scope.partial_count = 0
        $scope.importing = false
        $scope.saving = false
    }
    reset_scope()

    $scope.last_result_message = ''
    $scope.last_result_error = false


    const apply = debounce(() => $scope.$apply(), 100)

    function import_yield(datum, last_timestamp) {
        if (!import_yield.list) {
            import_yield.list = []
            import_yield.has_corrupt_data = false
        }
        if (datum !== true) {
            $scope.partial_count++
            $scope.progress = Math.ceil(100 * $scope.partial_count / $scope.new_record_count)
            apply()
            if (last_timestamp.isAfter(datum.timestamp)) {
                return import_yield.has_corrupt_data = true
            }
            return import_yield.list.push(datum)
        }
        const data = import_yield.list
        import_yield.list = null

        console.log('finished importing')
        setTimeout(() => {
            $scope.progress = 0
            $scope.partial_count = 0
            $scope.importing = false
            save_data(data, import_yield.has_corrupt_data)
        }, 1000)
    }

    function save_data(data, has_corrupt_data) {
        $scope.saving = true
        $scope.progress = 0
        $scope.partial_count = 0
        console.log('begin save')
        $scope.$apply()
        console.log(data)
        const num_blocks = Math.ceil(data.length / 100)
        const block_data = []
        for (let i = 0; i < num_blocks; i++) {
            block_data.push(data.slice(100 * i, 100 * (i + 1)))
        }
        // try saving. then try zeroing arduino internal count. if any fails, rollback
        bookshelf.knex
            .transaction(t => {
                Promise.all(
                    block_data.map(datum => bookshelf.knex
                        .insert(datum)
                        .into('logrecords')
                        .transacting(t))
                )
                    .then(_ => new Promise((resolve, reject) => {
                        $scope.progress = 50
                        $scope.$apply()
                        console.log('waiting arduino response')
                        serialport.parser.command(Event.DELETE_DATA_REQUEST, acknowledge => {
                            if (acknowledge) {
                                $scope.progress = 100
                                $scope.new_record_count = 0
                                $scope.$apply()
                                return resolve()
                            }
                            return reject()
                        })
                    }))
                    .then(t.commit)
                    .catch(t.rollback)
            })
            .then(res => {
                console.log('sucesso')
                $scope.last_result_message = 'A última importação foi bem sucedida.' + (has_corrupt_data ? ' Mas houve medidas corrompidas, que foram descartadas.' : '')
                $scope.last_result_error = false
                $scope.saving = false
                events.broadcast(events.DATA_IMPORTED)
                apply()
            }, err => {
                console.log('erro: ')
                console.log(err)
                $scope.last_result_message = 'Houve erro durante a última importação !'
                $scope.last_result_error = true
                $scope.saving = false
                apply()
            })
    }

    $scope.import = function () {
        $scope.progress = 0
        $scope.importing = true
        fetch(knex => knex.orderBy('timestamp', 'DESC').limit(1).select('timestamp'))
            .then(last => {
                const last_timestamp = last[0].timestamp
                console.log('last_timestamp === ' + last_timestamp)
                serialport.parser.command(Event.LOG_DATA_REQUEST, datum => import_yield(datum, last_timestamp))
            })
    }
}])

mod.controller('StatusController', ['$scope', 'serialport', 'events', function ($scope, serialport, events) {
    $scope.is_connected = false
    events.on(events.SERIALPORT_UPDATE, () => {
        $scope.is_connected = serialport.is_connected
        $scope.$apply()
    })
    $scope.connect = function () {
        connector.probe((err, found) => {
            if (err || !found) {
                serialport.is_connected = false
                return serialport.publish()
            }
            connector.connect((arg_parser, arg_port) => {
                serialport.parser = arg_parser
                serialport.port = arg_port
                serialport.is_connected = true
                return serialport.publish()
            }, (err) => {
                console.log(err)
                serialport.is_connected = false
                return serialport.publish()
            })
        })
    }
    $scope.connect()
}])

mod.controller('VisualizationController', ['$scope', 'events', function ($scope, events) {

    function draw_daily_series(records) {
        records.forEach(record => record.timestamp = record.timestamp.valueOf())

        Highcharts.stockChart('ultima-semana-container', {
            chart: {
                events: {
                    load: function () {
                        /* this.setTitle(null, {
                            text: 'Built chart in ' + (new Date() - start) + 'ms'
                        }) */
                    }
                },
                zoomType: 'xy',
                panning: true,
                panKey: 'ctrl',
                resetZoomButton: {
                    position: {
                        align: 'left',
                        x: 0
                    },
                    relativeTo: 'chart'
                }
            },

            legend: {
                enabled: true
            },

            rangeSelector: {

                buttons: [{
                    type: 'day',
                    count: 3,
                    text: '3d'
                }, {
                    type: 'week',
                    count: 1,
                    text: '1s'
                }, {
                    type: 'month',
                    count: 1,
                    text: '1m'
                }, {
                    type: 'month',
                    count: 6,
                    text: '6m'
                }, {
                    type: 'year',
                    count: 1,
                    text: '1a'
                }, {
                    type: 'all',
                    text: 'Tudo'
                }],
                selected: 3
            },

            yAxis: [{
                startOnTick: true,
                endOnTick: true,
                title: {
                    text: 'Temperatura (°C)'
                }
            }, {
                startOnTick: true,
                endOnTick: true,
                title: {
                    text: 'Umidade (%)'
                },
                opposite: false
            }],

            xAxis: {
                ordinal: false
            },

            title: {
                text: 'Medições do Arduino'
            },

            series: [{
                name: 'Temperatura',
                data: records.map(record => [record.timestamp, record.temperature]).reduce((acc, curr, i, arr) => {
                    acc.push(curr)
                    if (i + 1 < arr.length && (arr[i + 1][0] - arr[i][0]) > 15 * 60 * 1000) {
                        acc.push([arr[i][0] + 15 * 60 * 1000, null])
                    }
                    return acc
                }, []),
                tooltip: {
                    valueDecimals: 1,
                    valueSuffix: '°C'
                },
                yAxis: 0
            }, {
                name: 'Umidade',
                data: records.map(record => [record.timestamp, record.humidity]).reduce((acc, curr, i, arr) => {
                    acc.push(curr)
                    if (i + 1 < arr.length && (arr[i + 1][0] - arr[i][0]) > 15 * 60 * 1000) {
                        acc.push([arr[i][0] + 15 * 60 * 1000, null])
                    }
                    return acc
                }, []),
                tooltip: {
                    valueDecimals: 1,
                    valueSuffix: '%'
                },
                yAxis: 1
            }]

        }, function (chart) {
            chart.yAxis[0].startingExtremes = chart.yAxis[1].getExtremes();
        });
    }


    fetch(knex => knex
        /*  .where(function () {
             this.where('timestamp', '>', moment(new Date()).subtract(1, 'week').startOf('day').toDate())
         }) */
        .orderBy('timestamp')
    )
        .then(records => {
            draw_daily_series(records)
        })

    events.on(events.DATA_IMPORTED, () => {
        fetch(knex => knex.orderBy('timestamp'))
            .then(records => {
                draw_daily_series(records)
            })
    })

}])


