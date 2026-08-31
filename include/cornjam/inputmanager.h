#pragma once
#include <GLFW/glfw3.h>
#include <memory>

#include <misc/globals.h>

enum class Action {
    Jump,
    MoveUp,
    MoveDown,
    MoveLeft,
    MoveRight,
    Pause,
    Count
};

class InputManager {
    private:
    float _KeyTimer[GLFW_KEY_LAST + 1];
    int _ActionKeys[static_cast<int>(Action::Count)];
    const float c_MaximumTime = 1.0;
    float _RecentPressTimer = 0.1;
    public:

    InputManager(float maximumTime, float recentPress) : c_MaximumTime(maximumTime), _RecentPressTimer(recentPress) {
        for (int i = 0; i <= GLFW_KEY_LAST; i++) {
            _KeyTimer[i] = -0.001f;
        }
    }

    void Update(GLFWwindow *window) {

        for(int i = 0; i <= GLFW_KEY_LAST; i++) {
            int isKeyPressed = glfwGetKey(window, i);

            if(isKeyPressed == GLFW_PRESS) { //POSITIVE

                if (_KeyTimer[i] > 0) //Was it already pressed in the previous frame?
                    _KeyTimer[i] += DeltaTime;
                else
                    _KeyTimer[i] = DeltaTime;

                if (_KeyTimer[i] > c_MaximumTime)
                    _KeyTimer[i] = c_MaximumTime;

            } else { //NEGATIVE

                if (_KeyTimer[i] < 0) //Was it already released in the previous frame?
                    _KeyTimer[i] -= DeltaTime;
                else
                    _KeyTimer[i] = -DeltaTime;

                if (_KeyTimer[i] < c_MaximumTime)
                    _KeyTimer[i] = -c_MaximumTime;

            }

        }

    }

    void EditRecentPressTimer(float recentPress) {
        _RecentPressTimer = recentPress;
    }

    void BindAction(Action action, int Key) {
        _ActionKeys[int(action)] = Key;
    }

    //Keys

    inline float GetKeyTime(int Key) const { //The longer a key is pressed, the higher the number
        return _KeyTimer[Key];
    }

    inline bool isKeyPressed(int Key) const {
        return (_KeyTimer[Key] > 0);
    }

    inline bool isKeyReleased(int Key) const {
        return (_KeyTimer[Key] < 0);
    }

    inline bool isKeyPressedWithin(int Key, float Time) const { //Checks whether the key was pressed within Time or shorter
        return(_KeyTimer[Key] < Time && _KeyTimer[Key] > 0);
    }

    inline bool isKeyReleasedWithin(int Key, float Time) const { //Checks whether the key was released within Time or shorter
        return(_KeyTimer[Key] > -Time && _KeyTimer[Key] < 0);
    }

    inline bool isKeyPressedRecently(int Key) const {
        return(_KeyTimer[Key] < _RecentPressTimer && _KeyTimer[Key] > 0);
    }

    inline bool isKeyReleasedRecently(int Key) const {
        return(_KeyTimer[Key] > -_RecentPressTimer && _KeyTimer[Key] < 0);
    }

    inline bool isKeyPressedFor(int Key, float Time) const { //Checks whether the key was pressed for Time or longer
        return(_KeyTimer[Key] > Time);
    }

    inline bool isKeyReleasedFor(int Key, float Time) const { //Checks whether the key was released for Time or longer
        return(_KeyTimer[Key] < -Time);
    }

    //Actions

    inline float GetActionTime(Action action) const { //The longer a key is pressed, the higher the number
        return _KeyTimer[_ActionKeys[int(action)]];
    }

    inline bool isActionPressed(Action action) const {
        return (_KeyTimer[_ActionKeys[int(action)]] > 0);
    }

    inline bool isActionReleased(Action action) const {
        return (_KeyTimer[_ActionKeys[int(action)]] < 0);
    }

    inline bool isActionPressedWithin(Action action, float Time) const { //Checks whether the key was pressed within Time or shorter
        return(_KeyTimer[_ActionKeys[int(action)]] < Time && _KeyTimer[_ActionKeys[int(action)]] > 0);
    }

    inline bool isActionReleasedWithin(Action action, float Time) const { //Checks whether the key was released within Time or shorter
        return(_KeyTimer[_ActionKeys[int(action)]] > -Time && _KeyTimer[_ActionKeys[int(action)]] < 0);
    }

    inline bool isActionPressedRecently(Action action) const { //Checks whether the key was pressed within Time or shorter
        return(_KeyTimer[_ActionKeys[int(action)]] < _RecentPressTimer && _KeyTimer[_ActionKeys[int(action)]] > 0);
    }

    inline bool isActionReleasedRecently(Action action) const { //Checks whether the key was released within Time or shorter
        return(_KeyTimer[_ActionKeys[int(action)]] > -_RecentPressTimer && _KeyTimer[_ActionKeys[int(action)]] < 0);
    }

    inline bool isActionPressedFor(Action action, float Time) const { //Checks whether the key was pressed for Time or longer
        return(_KeyTimer[_ActionKeys[int(action)]] > Time);
    }

    inline bool isActionReleasedFor(Action action, float Time) const { //Checks whether the key was released for Time or longer
        return(_KeyTimer[_ActionKeys[int(action)]] < -Time);
    }
};