//
// Created by phage on 10/22/25.
//

#include "FpsCamera.hh"
#include <Engine.hh>

NAMESPACE_BEGIN
extern Engine* objptrGameEngine;

void FPSCamera::UpdateCamera(float dt)
{

    float hoffset = inputmgr->getMouseDeltaX();
    float voffset = -inputmgr->getMouseDeltaY();
    // Update camera angles using mouse offsets
    if (hoffset != 0.0f || voffset != 0.0f) {
        // Apply sensitivity and deadzone
        float pitchOffset = voffset * sensitivity.y;
        float yawOffset = hoffset * sensitivity.x;

        if (enableDeadZone) {
            if (fabs(pitchOffset) < deadZoneY) pitchOffset = 0.0f;
            if (fabs(yawOffset) < deadZoneX) yawOffset = 0.0f;
        }

        pitchNum += pitchOffset;
        pitchNum = glm::clamp(pitchNum, PITCHDOWN_LIMIT, PITCHUP_LIMIT);
        yawNum += yawOffset;
        yawNum = fmod(yawNum, 360.0f);
    }

    // 键盘持续输入
    glm::vec3 forward = glm::normalize(cameraDirection);
    glm::vec3 right = glm::normalize(glm::cross(forward, cameraUp));
    glm::vec3 up = cameraUp;

    glm::vec3 moveDirection(0.0f);
    if (inputmgr->isKeyHeld(SDL_SCANCODE_W)) moveDirection += forward;
    if (inputmgr->isKeyHeld(SDL_SCANCODE_S)) moveDirection -= forward;
    if (inputmgr->isKeyHeld(SDL_SCANCODE_A)) moveDirection -= right;
    if (inputmgr->isKeyHeld(SDL_SCANCODE_D)) moveDirection += right;
    if (inputmgr->isKeyHeld(SDL_SCANCODE_SPACE)) moveDirection += up;
    if (inputmgr->isKeyHeld(SDL_SCANCODE_LCTRL)) moveDirection -= up;

    if (glm::length(moveDirection) > 0.0f)
        position += glm::normalize(moveDirection) * cameraSpeed * dt;

    // Calculate new direction from pitch and yaw
    //in case you forget: the first two line calculate the x, z plane based on yaw,
    //pitch is difference between current cameraDirection and 0,0,-1, rotate towards y+ is positive
    //yaw is difference between cameraDirection and 0,0,-1, rotate towards x+ is positive
    cameraDirection = glm::vec3{
        sin(glm::radians(yawNum)) * cos(glm::radians(pitchNum)),
        sin(glm::radians(pitchNum)),
        -cos(glm::radians(yawNum)) * cos(glm::radians(pitchNum))
    };
    //don't forget update camera up, using pitch and yaw
    cameraUp = glm::vec3{
        -sin(glm::radians(yawNum)) * sin(glm::radians(pitchNum)),
        cos(glm::radians(pitchNum)),
        cos(glm::radians(yawNum)) * sin(glm::radians(pitchNum))
    };


    // Update view matrix
    viewMatrix = glm::lookAt(position, position + cameraDirection, cameraUp);
    // Optionally update projection matrix if FOV changes
    if (project_dirty) {
        ENGINE_DEBUG("Re-construct camera based on window size: {}, {}", objptrAppContext->aRenderContext->windowwidth, objptrAppContext->aRenderContext->windowheight);
        float aspect_ratio = static_cast<float>(objptrAppContext->aRenderContext->windowwidth) / static_cast<float>(objptrAppContext->aRenderContext->windowheight);
        setCameraProjectionMatrix(glm::perspective(
            glm::radians(degreeFOV),
            aspect_ratio,
            near, far
        ));
        project_dirty = false;
    }
}

FPSCamera::FPSCamera(glm::vec3 position, float angle) :
Camera(position, angle)
{
    this->inputmgr = objptrGameEngine->getInputManager();
}


NAMESPACE_END
