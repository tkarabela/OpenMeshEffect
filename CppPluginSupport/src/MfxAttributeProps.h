#pragma once

enum class MfxAttributeType {
    Unknown = -1,
    UByte,
    Int,
    Float,
};

/**
 * Mfx*Props classes are a little different: for caching and convenience, they
 * store some data and care must be taken not to copy it around too much
 * (though it does not directly store the attribute data, only metadata).
 */
struct MfxAttributeProps
{
    MfxAttributeProps() : type(MfxAttributeType::Unknown), stride(0), componentCount(0), data(NULL), isOwner(false) {}

    MfxAttributeType type;
    int stride;
    int componentCount;
    char* data;
    bool isOwner;

    /**
     * Get value at particular index.
     */
    bool GetValue(int *value, int i, int component = 0) {
        switch (type) {
            case MfxAttributeType::UByte: *value = ((unsigned char*)(data + i*stride))[component]; return true;
            case MfxAttributeType::Int: *value = ((int*)(data + i*stride))[component]; return true;
            case MfxAttributeType::Float: break;
            case MfxAttributeType::Unknown: break;
        }
        return false;
    }
    bool GetValue(float *value, int i, int component = 0) {
        switch (type) {
            case MfxAttributeType::UByte: *value = ((unsigned char*)(data + i*stride))[component]; return true;
            case MfxAttributeType::Int: *value = ((int*)(data + i*stride))[component]; return true;
            case MfxAttributeType::Float: *value = ((float*)(data + i*stride))[component]; return true;
            case MfxAttributeType::Unknown: break;
        }
        return false;
    }

    /**
     * Set value at particular index.
     */
    bool SetValue(int value, int i, int component = 0) {
        switch (type) {
            case MfxAttributeType::UByte: ((unsigned char*)(data + i*stride))[component] = value; return true;
            case MfxAttributeType::Int: ((int*)(data + i*stride))[component] = value; return true;
            case MfxAttributeType::Float: ((float*)(data + i*stride))[component] = value; return true;
            case MfxAttributeType::Unknown: break;
        }
        return false;
    }
    bool SetValue(float value, int i, int component = 0) {
        switch (type) {
            case MfxAttributeType::UByte: break;
            case MfxAttributeType::Int: break;
            case MfxAttributeType::Float: ((float*)(data + i*stride))[component] = value; return true;
            case MfxAttributeType::Unknown: break;
        }
        return false;
    }
};
