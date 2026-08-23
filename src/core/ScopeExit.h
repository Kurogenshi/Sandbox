#pragma once

#include "core/Macros.h"

#include <type_traits>
#include <utility>

namespace sandbox {

template <typename Fn>
class ScopeExit
{
public:
    static_assert(std::is_nothrow_invocable_v<Fn>, "A scope exit action must not throw: it runs from a destructor.");

    constexpr explicit ScopeExit(Fn fn) noexcept(std::is_nothrow_move_constructible_v<Fn>) : m_fn(std::move(fn))
    {
    }

    ~ScopeExit() noexcept
    {
        if (m_active)
            m_fn();
    }

    ScopeExit(ScopeExit&& other) noexcept(std::is_nothrow_move_constructible_v<Fn>) : m_fn(std::move(other.m_fn)), m_active(std::exchange(other.m_active, false))
    {
    }

    ScopeExit(const ScopeExit&) = delete;
    ScopeExit& operator=(const ScopeExit&) = delete;
    ScopeExit& operator=(ScopeExit&&) = delete;

    void release() noexcept { m_active = false; }

    [[nodiscard]] bool active() const noexcept { return m_active; }

private:
    Fn m_fn;
    bool m_active = true;
};

template <typename Fn>
ScopeExit(Fn) -> ScopeExit<Fn>;

namespace detail {

struct ScopeExitTag
{
};

template <typename Fn>
ScopeExit<Fn> operator+(ScopeExitTag, Fn&& fn)
{
    return ScopeExit<Fn>{std::forward<Fn>(fn)};
}

}
}

#define SANDBOX_SCOPE_EXIT                                 \
    auto SANDBOX_UNIQUE_NAME(sandboxScopeExit_) =          \
        ::sandbox::detail::ScopeExitTag{} + [&]() noexcept