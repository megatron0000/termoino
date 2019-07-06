const connector = require('bau-arduino-data-logging').connector
const Event = require('bau-arduino-data-logging').Event
const path = require('path')
const bookshelf = require('./webapp/database').bookshelf

// connector.connect((parser, port) => {
//     parser.command(Event.LOG_DATA_REQUEST, (data) => {
//         console.log('info: log data received')
//     })
//     parser.command(Event.DELETE_DATA_REQUEST, (result) => {
//         console.log('info: delete data ' + result)
//     })
//     parser.command(Event.KEEPALIVE_REQUEST, (result) => {
//         console.log('info: keepalive ' + result)
//     })
//     parser.command(Event.CLOSE_REQUEST, (result) => {
//         port.close((err) => {
//             if (err) {
//                 console.log('error: ' + err)
//             }
//             console.log('info: closed connection')
//         })
//     })
// }, (err) => {
//     console.log('error: ' + err)
// })

// Modules to control application life and create native browser window
const { app, BrowserWindow } = require('electron')

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow() {
    // Create the browser window.
    mainWindow = new BrowserWindow({
        width: 800, height: 600, webPreferences: {
            webSecurity: false
        }
    })

    // mainWindow.setMenu(null);


    // and load the index.html of the app.
    mainWindow.loadFile(path.join(__dirname, 'webapp', 'index.html'))

    // Open the DevTools.
    // mainWindow.webContents.openDevTools()

    // Emitted when the window is closed.
    mainWindow.on('closed', function () {
        bookshelf.knex.destroy().then(() => console.log('destroyed'))
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        mainWindow = null
    })
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// Quit when all windows are closed.
app.on('window-all-closed', function () {
    // On macOS it is common for applications and their menu bar
    // to stay active until the user quits explicitly with Cmd + Q
    if (process.platform !== 'darwin') {
        app.quit()
    }
})

app.on('activate', function () {
    // On macOS it's common to re-create a window in the app when the
    // dock icon is clicked and there are no other windows open.
    if (mainWindow === null) {
        createWindow()
    }
})

// In this file you can include the rest of your app's specific main process
// code. You can also put them in separate files and require them here.
