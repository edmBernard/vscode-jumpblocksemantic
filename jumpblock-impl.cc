#include <assert.h>
#include <napi.h>

#include <filesystem>
#include <iostream>
#include <string>
#include <tree_sitter/api.h>

extern "C" {
// Declare the `tree_sitter_cpp` function, which is
// implemented by the `tree-sitter-cpp` library.
const TSLanguage *tree_sitter_cpp(void);
const TSLanguage *tree_sitter_python(void);
}

namespace fs = std::filesystem;

namespace {

enum class Direction {
  Down,
  Up,
  Out
};

bool equal(const TSPoint &pt1, const TSPoint &pt2) {
  return pt1.row == pt2.row && pt1.column == pt2.column;
}

bool equalPosition(const TSNode& node1, const TSNode& node2) {
  TSPoint start1 = ts_node_start_point(node1);
  TSPoint start2 = ts_node_start_point(node2);
  TSPoint end1 = ts_node_end_point(node1);
  TSPoint end2 = ts_node_end_point(node2);
  return equal(start1, start2) && equal(end1, end2);
}

void moveOut(TSNode& node) {
  TSNode parentNode = ts_node_parent(node);
  // if no parent it means we alread are at the root node
  if (ts_node_is_null(parentNode)) {
    return;
  }

  // there is an issue on type_qualifier node. The jump to parent don't move
  // So we will search for the parent of the previous sibling first
  // if there is no suitable previous sibling, we search for the next sibling
  if (!equalPosition(parentNode, node)) {
    node = parentNode;
    return;
  }

  TSNode prevNode = ts_node_prev_sibling(parentNode);
  if (!ts_node_is_null(prevNode) && !equalPosition(prevNode, parentNode)) {
    parentNode = ts_node_parent(prevNode);
    if (!ts_node_is_null(parentNode)) {
      node = parentNode;
      return;
    }
  }

  TSNode nextNode = ts_node_next_sibling(parentNode);
  if (!ts_node_is_null(nextNode) && !equalPosition(nextNode, parentNode)) {
    parentNode = ts_node_parent(nextNode);
    if (!ts_node_is_null(parentNode)) {
      node = parentNode;
      return;
    }
  }
}

void moveUp(TSNode& node) {
  TSNode prevNode = ts_node_prev_sibling(node);
  if (!ts_node_is_null(prevNode) && !equalPosition(prevNode, node)) {
    node = prevNode;
    return;
  }
  // if search for previous don't move we check for parent
  moveOut(node);
}

void moveDown(TSNode& node) {
  TSNode nextNode = ts_node_next_sibling(node);
  if (!ts_node_is_null(nextNode) && !equalPosition(nextNode, node)) {
    node = nextNode;
    return;
  }
  // if search for next don't move we check for parent
  moveOut(node);
}

} // namespace

template <Direction D>
Napi::Object JumpOut(const Napi::CallbackInfo &info) {
  Napi::Env env = info.Env();
  Napi::Object obj = Napi::Object::New(env);
  // TODO: I should not do that
  obj.Set(Napi::String::New(env, "row"), 0);
  obj.Set(Napi::String::New(env, "col"), 0);
  obj.Set(Napi::String::New(env, "selection_start_row"), 0);
  obj.Set(Napi::String::New(env, "selection_start_col"), 0);
  obj.Set(Napi::String::New(env, "selection_end_row"), 0);
  obj.Set(Napi::String::New(env, "selection_end_col"), 0);

  // args : filename, data, startX, startY, endX, endY
  if (info.Length() != 6) {
    Napi::TypeError::New(env, "Wrong number of arguments").ThrowAsJavaScriptException();
    return obj;
  }

  if (!info[0].IsString() || !info[1].IsString() || !info[2].IsNumber() || !info[3].IsNumber() || !info[4].IsNumber() || !info[5].IsNumber()) {
    Napi::TypeError::New(env, "Wrong arguments type").ThrowAsJavaScriptException();
    return obj;
  }

  std::string languageId = info[0].As<Napi::String>();
  std::string source = info[1].As<Napi::String>();

  // Selection is done between active (cursor) and anchor
  const TSPoint activePoint{info[2].As<Napi::Number>().Uint32Value(), info[3].As<Napi::Number>().Uint32Value()};
  const TSPoint anchorPoint{info[4].As<Napi::Number>().Uint32Value(), info[5].As<Napi::Number>().Uint32Value()};

  // Create a parser.
  TSParser *parser = ts_parser_new();

  // Set the parser's language
  if (languageId == "cpp") {
    ts_parser_set_language(parser, tree_sitter_cpp());
  } else if (languageId == "python") {
    ts_parser_set_language(parser, tree_sitter_python());
  } else {
    Napi::TypeError::New(env, "Unsupported language").ThrowAsJavaScriptException();
    return obj;
  }

  // Build a syntax tree based on source code stored in a string.
  TSTree *tree = ts_parser_parse_string(parser, nullptr, source.data(), source.size());

  // Get the root node of the syntax tree.
  TSNode root_node = ts_tree_root_node(tree);

  // Get smallest node that contain the selection
  TSNode currentNode = ts_node_descendant_for_point_range(root_node, activePoint, anchorPoint);
  if (ts_node_is_null(currentNode)) {
    Napi::TypeError::New(env, "Can't find node for this selection").ThrowAsJavaScriptException();
    return obj;
  }

  switch (D) {
  case Direction::Up:
    moveUp(currentNode);
    break;
  case Direction::Down:
    moveDown(currentNode);
    break;
  case Direction::Out:
    moveOut(currentNode);
    break;
  }

  TSPoint selectionStart = ts_node_start_point(currentNode);
  obj.Set(Napi::String::New(env, "selection_start_row"), selectionStart.row);
  obj.Set(Napi::String::New(env, "selection_start_col"), selectionStart.column);

  TSPoint selectionEnd = ts_node_end_point(currentNode);
  obj.Set(Napi::String::New(env, "selection_end_row"), selectionEnd.row);
  obj.Set(Napi::String::New(env, "selection_end_col"), selectionEnd.column);

  ts_tree_delete(tree);
  ts_parser_delete(parser);

  return obj;
}

Napi::Object Init(Napi::Env env, Napi::Object exports) {
  exports.Set(Napi::String::New(env, "jumpUp"), Napi::Function::New(env, JumpOut<Direction::Up>));
  exports.Set(Napi::String::New(env, "jumpDown"), Napi::Function::New(env, JumpOut<Direction::Down>));
  exports.Set(Napi::String::New(env, "jumpOut"), Napi::Function::New(env, JumpOut<Direction::Out>));
  return exports;
}

NODE_API_MODULE(jumpblock - impl, Init)
