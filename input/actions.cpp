#include "actions.h"

#include "keyboard.h"

#include <algorithm>
#include <array>
#include <cstddef>

namespace eclipse::input {
namespace {

using Bindings = std::array<std::vector<int>, static_cast<std::size_t>(Action::Count)>;

Bindings bindings;

std::size_t Index(Action action) {
  return static_cast<std::size_t>(action);
}

bool ValidAction(Action action) {
  return Index(action) < bindings.size();
}

template <typename Query>
bool QueryAction(Action action, Query query) {
  if (!ValidAction(action)) {
    return false;
  }
  return std::any_of(bindings[Index(action)].begin(), bindings[Index(action)].end(),
                     query);
}

} // namespace

void InitializeActions() { ResetActionBindings(); }

void ResetActionBindings() {
  for (auto& actionBindings : bindings) {
    actionBindings.clear();
  }

  BindKey(Action::MoveUp, ECLIPSE_INPUT_KEY_W);
  BindKey(Action::MoveUp, ECLIPSE_INPUT_KEY_UP);
  BindKey(Action::MoveDown, ECLIPSE_INPUT_KEY_S);
  BindKey(Action::MoveDown, ECLIPSE_INPUT_KEY_DOWN);
  BindKey(Action::MoveLeft, ECLIPSE_INPUT_KEY_A);
  BindKey(Action::MoveLeft, ECLIPSE_INPUT_KEY_LEFT);
  BindKey(Action::MoveRight, ECLIPSE_INPUT_KEY_D);
  BindKey(Action::MoveRight, ECLIPSE_INPUT_KEY_RIGHT);
  BindKey(Action::Jump, ECLIPSE_INPUT_KEY_SPACE);
  BindKey(Action::Pause, ECLIPSE_INPUT_KEY_ESCAPE);
  BindKey(Action::Interact, ECLIPSE_INPUT_KEY_E);
}

bool BindKey(Action action, int key) {
  if (!ValidAction(action) || !keyboard::IsValidKey(key)) {
    return false;
  }
  auto& actionBindings = bindings[Index(action)];
  if (std::find(actionBindings.begin(), actionBindings.end(), key) ==
      actionBindings.end()) {
    actionBindings.push_back(key);
  }
  return true;
}

bool UnbindKey(Action action, int key) {
  if (!ValidAction(action)) {
    return false;
  }
  auto& actionBindings = bindings[Index(action)];
  const auto it = std::find(actionBindings.begin(), actionBindings.end(), key);
  if (it == actionBindings.end()) {
    return false;
  }
  actionBindings.erase(it);
  return true;
}

void ClearActionBindings(Action action) {
  if (ValidAction(action)) {
    bindings[Index(action)].clear();
  }
}

const std::vector<int>& GetActionBindings(Action action) {
  static const std::vector<int> empty;
  return ValidAction(action) ? bindings[Index(action)] : empty;
}

bool ActionDown(Action action) {
  return QueryAction(action, [](int key) { return keyboard::Key(key); });
}

bool ActionPressed(Action action) {
  return QueryAction(action, [](int key) { return keyboard::KeyDown(key); });
}

bool ActionReleased(Action action) {
  return QueryAction(action, [](int key) { return keyboard::KeyUp(key); });
}

} // namespace eclipse::input
