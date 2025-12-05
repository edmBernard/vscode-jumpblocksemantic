# Jump Block Semantic

JumpBlock is a simple VSCode extension to navigate files by semantic block.
We use [tree-sitter](https://tree-sitter.github.io/tree-sitter/) to parse the code.
We currently only support C++ and Python

Inpired by [jumpblock](https://github.com/alexnaraghi/vscode-jumpblock) extension that allow to jump between empty line.
Having semantic block is not always better than jumping by empty line.

## Installation

### Get source code

tree-sitter are download as a submodule. don't forget to initialize them

```
git clone https://github.com/edmBernard/vscode-jumpblocksemantic.git
git submodule update --init --recursive
```

### Build
```
npm install
```

### Create the vsix package for vscode

```
vsce package
```

### Installation in VSCode

In `Extension > ... > Install from VSIX` and select the package generated

## Features

Navigating by semantic block.

Currently there's just these shortcuts:
jumpBlockSemantic.up : move to previous node and move to parent if there is no previous node at the same level
jumpBlockSemantic.down : move to next node and move to parent if there is no next node at the same level
jumpBlockSemantic.out : move to parent node

I like to bind them to ctrl+up and ctrl+down so I can hold the ctrl key down to navigate left and right by word, and up and down by block.

Ex.
```json
{
    "key": "ctrl+up",
    "command": "jumpBlockSemantic.up"
},
{
    "key": "ctrl+down",
    "command": "jumpBlockSemantic.down"
},
{
    "key": "ctrl+left",
    "command": "jumpBlockSemantic.out"
}
```

## Release Notes

### 0.0.2
- Add Zig support
- Update dependencies

### 0.0.1
- Initial push of JumpBlockSemantic
