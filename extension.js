// The module 'vscode' contains the VS Code extensibility API
// Import the module and reference it with the alias vscode in your code below
var vscode = require('vscode');
var addon = require('bindings')('jumpblock-impl');

// cheap enum
const Direction = {
    Down: 0,
    Up: 1,
    Out: 2,
}

// this method is called when your extension is activated
// your extension is activated the very first time the command is executed
function activate(context) {
    var disposable = vscode.commands.registerCommand('jumpBlockSemantic.down', function () {
        jump(Direction.Down);
    });

    var disposable = vscode.commands.registerCommand('jumpBlockSemantic.up', function () {
       jump(Direction.Up);
    });

    var disposable = vscode.commands.registerCommand('jumpBlockSemantic.out', function () {
       jump(Direction.Out);
    });

    context.subscriptions.push(disposable);
}
exports.activate = activate;

// this method is called when your extension is deactivated
function deactivate() {
}
exports.deactivate = deactivate;

function jump(direction)
{
    const editor = vscode.window.activeTextEditor;
    const document = editor.document;
    const position = editor.selection.start;
    const anchor = editor.selection.end;

    var point = {};
    var selectionStart = {};
    var selectionEnd = {};
    if (direction == Direction.Down) {
        point = addon.jumpDown(document.languageId, document.getText(null), position.line, position.character, anchor.line, anchor.character);
    } else if (direction == Direction.Up) {
        point = addon.jumpUp(document.languageId, document.getText(null), position.line, position.character, anchor.line, anchor.character);
    } else if (direction == Direction.Out) {
        point = addon.jumpOut(document.languageId, document.getText(null), position.line, position.character, anchor.line, anchor.character);
    }
    selectionStart = new vscode.Position(point.selection_start_row, point.selection_start_col);
    selectionEnd = new vscode.Position(point.selection_end_row, point.selection_end_col);

    var newSelection = new vscode.Selection(selectionEnd, selectionStart);
    editor.selection = newSelection;
    editor.revealRange(new vscode.Range(selectionStart, selectionStart));
}
