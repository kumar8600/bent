#pragma once

struct unko
{
    unko() :
        state(DEFAULT_CONSTRUCTED)
    {}

    unko(const unko&) :
        state(COPY_CONSTRUCTED)
    {}

    unko(unko&&) :
        state(MOVE_CONSTRUCTED)
    {}

    ~unko()
    {
        state = DESTRUCTED;
    }

    enum state
    {
        DEFAULT_CONSTRUCTED,
        COPY_CONSTRUCTED,
        MOVE_CONSTRUCTED,
        DESTRUCTED
    };

    state state;
};
