#include <IRandom.h>

#include <queue>

class FakeRandom : public IRandom
{
public:
    void addInt(int value)
    {
        _values.push(value);
    }

    int32_t next(int32_t, int32_t) override
    {
        int32_t value = _values.front();
        _values.pop();
        return value;
    }

private:
    std::queue<int32_t> _values;
};