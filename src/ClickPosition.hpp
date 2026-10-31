#pragma once

namespace phosophurs::pathfinder {
    struct MouseOffset {
        float x = 0.f;
        float y = 0.f;
    };

    // Returns a bounded random offset for a non-frame-perfect mouse click.
    // The pathfinder must add this to its intended mouse coordinate immediately
    // before dispatching the click. Keyboard inputs should not call this.
    // A frame-perfect click always receives {0, 0}.
    MouseOffset getHumanisedMouseOffset(bool framePerfect);
}
