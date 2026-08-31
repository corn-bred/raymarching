#pragma once
#include <AL/al.h>
#include <AL/alc.h>

#include <iostream>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <cornbreadlib/audiodata.h>

#define _SOURCESIZE 32

class AudioManager {
    
    public:
    ALCcontext *AudioContext = nullptr;
    ALCdevice *AudioDevice = nullptr;

    std::vector<ALuint> SourceAvailPool;
    std::vector<ALuint> SourceUsedPool;

    AudioManager() {
        AudioDevice = alcOpenDevice(NULL);
        if (!AudioDevice) {
            std::cerr << "Audio device creation failure\n";
            return;
        }

        AudioContext = alcCreateContext(AudioDevice, NULL);
        if (!AudioContext) {
            std::cerr << "Audio context creation failure\n";
            return;
        }

        alcMakeContextCurrent(AudioContext);

        SourceAvailPool.reserve(_SOURCESIZE);
        SourceUsedPool.reserve(_SOURCESIZE);

        for(int i = 0; i < _SOURCESIZE; i++) {
            ALuint src;
            alGenSources(1, &src);
            SourceAvailPool.push_back(src);
        }
    }

    ~AudioManager() {
        alcMakeContextCurrent(0);
        alcDestroyContext(AudioContext);
        alcCloseDevice(AudioDevice);
    }

    AudioManager(AudioManager &&other) : 
    AudioContext(other.AudioContext), AudioDevice(other.AudioDevice), 
    SourceAvailPool(std::move(other.SourceAvailPool)), SourceUsedPool(std::move(other.SourceUsedPool)) 
    {
        other.AudioContext = nullptr;
        other.AudioDevice = nullptr;
    }

    AudioManager &operator=(AudioManager &&other) {
        if (this != &other) {
            if (AudioContext != nullptr) {
                alcMakeContextCurrent(0);
                alcDestroyContext(AudioContext);
                AudioContext = nullptr;
            }
            if (AudioDevice != nullptr) {
                alcCloseDevice(AudioDevice);
                AudioDevice = nullptr;
            }
            AudioContext = other.AudioContext;
            AudioDevice = other.AudioDevice;

            SourceAvailPool = std::move(other.SourceAvailPool);
            SourceUsedPool = std::move(other.SourceUsedPool); 

            other.AudioContext = nullptr;
            other.AudioDevice = nullptr;
        }
        return *this;
    }

    AudioManager(AudioManager &other) = delete;
    AudioManager operator=(AudioManager &other) = delete;

    void PlaySound(AudioData &buffer) {
        if (SourceAvailPool.empty()) return;

        ALuint src = SourceAvailPool.back();
        SourceAvailPool.pop_back();

        SourceUsedPool.push_back(src);

        alSourcei(src, AL_BUFFER, buffer.ID);

        alSource3f(src, AL_POSITION, buffer.Position.x, buffer.Position.y, buffer.Position.z);
        alSourcei(src, AL_LOOPING, buffer.Looping);
        alSourcef(src, AL_PITCH, buffer.Pitch);
        alSourcef(src, AL_GAIN, buffer.Gain);
        alSource3f(src, AL_VELOCITY, buffer.Velocity.x, buffer.Velocity.y, buffer.Velocity.z);
        alSource3f(src, AL_DIRECTION, buffer.NormalizedVec.x, buffer.NormalizedVec.y, buffer.NormalizedVec.z);

        alSourcePlay(src);
    }

    void Update() {
        for (auto iterator = SourceUsedPool.begin(); iterator != SourceUsedPool.end();) {
            ALuint src = *iterator; //give me the point
            ALint SourceState;
            alGetSourcei(src, AL_SOURCE_STATE, &SourceState);

            if(SourceState == AL_STOPPED) {
                SourceAvailPool.push_back(src);
                iterator = SourceUsedPool.erase(iterator); //returns the next iterator that comes after the previously deleted iterator
            } else {
                iterator++;
            }

        }
    }

    void SetListenerPosition(glm::vec3 Position) {
        alListener3f(AL_POSITION, Position.x, Position.y, Position.z);
    } 
    void SetListenerVelocity(glm::vec3 Velocity) {
        alListener3f(AL_VELOCITY, Velocity.x, Velocity.y, Velocity.z);
    }
    void SetListenerOrientation(glm::vec3 forward, glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f)) {
        ALfloat Orientation[6] = {
            forward.x, forward.y, forward.z,
            up.x, up.y, up.z
        };

        alListenerfv(AL_ORIENTATION, Orientation);
    }
    void SetListenerGain(float Gain) {
        alListenerf(AL_GAIN, Gain);
    }
};