const path = require('path')

const knex = require('knex')({
    client: 'sqlite3',
    debug: false,
    connection: {
        filename: path.resolve('./testdb.sqlite')
    },
    useNullAsDefault: true
})

const bookshelf = require('bookshelf')(knex)

// knex.schema.createTable('logrecords', builder => {
//     builder.increments()
//     builder.dateTime('timestamp')
//     builder.float('temperature')
//     builder.float('humidity')
//     console.log('created')
// }).then(console.log, console.log)

class LogRecord extends bookshelf.Model {
    get tableName() {
        return 'logrecords'
    }
}

// [10, 12, 14, 16].forEach(hour => {
//     let date = new Date('Sat, 19 Jan 2019 12:44:21 GMT')
//     date.setUTCHours(hour)
//     new LogRecord({ timestamp: date, temperature: 25.0 + hour, humidity: 80.5 }).save()
//         .then(doc => {
//             console.log('info: ')
//             console.log(doc)
//         }).catch(err => {
//             console.log('error: ')
//             console.error(err)
//         })
// })

class LogCollection extends bookshelf.Collection {
    get model() {
        return LogRecord
    }
}


exports.LogRecord = LogRecord
exports.LogCollection = LogCollection
exports.bookshelf = bookshelf
