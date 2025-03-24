#pragma once

template <typename Event>
class IMediator {
public:
    IMediator() noexcept = default;
    virtual ~IMediator() noexcept = default;

    virtual void Notify(const Event& event) noexcept = 0;
};
