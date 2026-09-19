#pragma once

#include <vector>

namespace eclipse::input {

enum class Action {
  MoveUp,
  MoveDown,
  MoveLeft,
  MoveRight,
  Jump,
  Pause,
  Interact,
  Count,
};

void InitializeActions();
void ResetActionBindings();
bool BindKey(Action action, int key);
bool UnbindKey(Action action, int key);
void ClearActionBindings(Action action);
const std::vector<int>& GetActionBindings(Action action);

bool ActionDown(Action action);
bool ActionPressed(Action action);
bool ActionReleased(Action action);

} // namespace eclipse::input
