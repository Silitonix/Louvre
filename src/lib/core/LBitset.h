#ifndef LBITSET_H
#define LBITSET_H

#include <LNamespaces.h>

/**
 * @brief Compact way of storing and managing conditions or states
 *
 * The LBitset class template is similar to `std::bitset` in that it enables the compact storage of a set of conditions or states using bits.\n
 * Unlike `std::bitset`, LBitset functions are designed to modify and retrieve bit states using flags rather than indices, which are ideally
 * defined within an enum.
 *
 * It is widely used in the private API of Louvre classes to optimize memory usage.
 *
 * Example usage:
 *
 * @code
 * enum MyFlags
 * {
 *      FlagA = 1 << 0,
 *      FlagB = 1 << 1,
 *      FlagC = 1 << 2
 * };
 *
 * LBitset<MyFlags> myFlags;
 *
 * myFlags.add(MyFlags::FlagA | MyFlags::FlagC);
 *
 * if (myFlags.check(MyFlags::FlagC))
 * {
 *      // do something
 * }
 * @endcode
 */
template <class T>
class Louvre::LBitset
{
    using Flag = typename std::conditional<
        std::is_enum<T>::value,
        typename std::underlying_type<T>::type,
        T
        >::type;

public:

    /**
     * @brief Constructor for LBitset
     *
     * Initializes the bitset with the specified initial flags. If no flags are provided, the bitset is constructed empty without any flags.
     *
     * @param flags Initial flags to set (default is 0)
     */
    LBitset(Flag flags = 0) noexcept : m_flags(flags) {}

    /**
     * @brief Add new flags to the bitfield
     *
     * Sets the specified flags by combining them with the existing flags using the bitwise OR operator '|'.
     *
     * @param flags The flag or combination of flags to be added
     */
    void add(Flag flags) noexcept
    {
        m_flags |= flags;
    }

    /**
     * @brief Remove flags from the bitfield
     *
     * Clears the specified flags by performing a bitwise AND operation with the complement of the provided flags.
     *
     * @param flags The flag or combination of flags to be removed
     */
    void remove(Flag flags) noexcept
    {
        m_flags &= ~flags;
    }

    /**
     * @brief Check if at least one flag exists
     *
     * Checks if at least one of the specified flags is set in the bitfield.
     *
     * @param flags The flag or combination of flags to be checked
     * @return `true` if at least one flag is set, otherwise `false`
     */
    bool check(Flag flags) const noexcept
    {
        return (m_flags & flags) != 0;
    }

    /**
     * @brief Check if all specified flags exist
     *
     * Checks if all of the specified flags are set in the bitfield.
     *
     * @param flags The flag or combination of flags to be checked
     * @return `true` if all specified flags are set, otherwise `false`
     */
    bool checkAll(Flag flags) const noexcept
    {
        return (m_flags & flags) == flags;
    }

    /**
     * @brief Get the current set of flags
     *
     * Retrieves the current set of flags stored in the bitfield.
     *
     * @return The current set of flags
     */
    Flag get() const noexcept
    {
        return m_flags;
    }

    /**
     * @brief Set new flags in the bitfield
     *
     * Replaces the current set of flags in the bitfield with the specified flags.
     *
     * @param flags The new flag or combination of flags to be set
     */
    void set(Flag flags) noexcept
    {
        m_flags = flags;
    }

    /**
     * @brief Set or unset a specific flag in the bitfield
     *
     * Modifies the specified flag in the bitfield. If `enable` is `true`, the flag is set, otherwise, it is removed.
     *
     * @param flag The flag to be set or unset
     * @param enable If `true`, set the flag, if `false`, remove the flag
     */
    void setFlag(Flag flag, bool enable) noexcept
    {
        if (enable)
            add(flag);
        else
            remove(flag);
    }

private:
    Flag m_flags;
};

#endif // LBITSET_H
