#ifndef ANIMATION_H
#define ANIMATION_H

class Animation {
  public:
    unsigned int index;          // index track of animation
    unsigned int numFrames;      // no of frames of the animation
    unsigned int animationSpeed; // these comments are obvious
    Animation();
    Animation(unsigned int index, unsigned int numFrames,
              unsigned int animationSpeed);
};

#endif
