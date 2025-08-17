#pragma once

#include <utility>
#include <cstddef>

namespace mc {
    template<size_t N, typename T, template <size_t, typename> typename U, typename = void>
    static constexpr size_t fieldIndex = 0;

    template<size_t N, typename T, template <size_t, typename> typename U>
    static constexpr size_t fieldIndex<N, T, U, std::void_t<decltype(U<N, T>::fieldName)>> = fieldIndex<N + 1, T, U> + 1;

    template<typename T, typename Tuple>
    constexpr bool tuple_contains_type = false;

    template<typename T, typename... Ts>
    constexpr bool tuple_contains_type<T, std::tuple<Ts...>> = std::disjunction_v<std::is_same<T, Ts>...>;

    template<typename T, typename Tuple, bool Found = tuple_contains_type<T, Tuple>>
    struct get_tuple_index_impl;

    template<typename T, typename Tuple>
    struct get_tuple_index_impl<T, Tuple, false> {
        static_assert(tuple_contains_type<T, Tuple>, "Type T not found in Tuple");
    };

    template<typename T, typename... Ts>
    struct get_tuple_index_impl<T, std::tuple<T, Ts...>, true> {
        static constexpr size_t value = 0;
    };

    template<typename T, typename U, typename... Ts>
    struct get_tuple_index_impl<T, std::tuple<U, Ts...>, true> {
        static constexpr size_t value = 1 + get_tuple_index_impl<T, std::tuple<Ts...>>::value;
    };

    template<typename T, typename Tuple>
    constexpr size_t get_tuple_index = get_tuple_index_impl<T, Tuple>::value;

    template<size_t... indices, class LoopBody>
    void loopImpl(std::index_sequence<indices...>, LoopBody &&loop_body) {
        (loop_body(std::integral_constant<size_t, indices>{}), ...);
    }

    template<size_t N, class LoopBody>
    void loop(LoopBody &&loop_body) {
        loopImpl(std::make_index_sequence<N>{}, std::forward<LoopBody>(loop_body));
    }
}

#define CONFIG_FIELD(Type, Name, Category, DefaultValue)                                                           \
    static constexpr auto Name##_category = Category;                                                              \
    using Name##_type = Type;                                                                                      \
    Name##_type Name = DefaultValue;                                                                               \
    template <size_t, typename> struct FieldData;                                                                  \
    static constexpr size_t Name##_index = mc::fieldIndex<0, struct Dummy##Name, FieldData>;                       \
                                                                                                                   \
    template <typename T>                                                                                          \
    struct FieldData<Name##_index, T> {                                                                            \
        static constexpr auto fieldName = #Name;                                                                   \
        static constexpr auto fieldOptional = true;                                                                \
        using fieldType = Type;                                                                                    \
        static constexpr auto fieldCategory = Category;                                                            \
                                                                                                                   \
        template <typename Parent> static constexpr fieldType                                                      \
        Parent::*fieldPtr = &Parent::Name;                                                                         \
    };                                                                                                             \
static_assert(true, "Forcing ;")

#define NAMED_FIELD_SET(Type, Name)                                                           \
    using Name##_type = Type;                                                                 \
    template <size_t, typename> struct FieldData;                                             \
    static constexpr size_t Name##_index = mc::fieldIndex<0, struct Dummy##Name, FieldData>;  \
                                                                                              \
    template <typename T>                                                                     \
    struct FieldData<Name##_index, T> {                                                       \
        static constexpr auto fieldName = #Name;                                              \
        static constexpr auto fieldOptional = Name##_optional;                                \
        using fieldType = Type;                                                               \
                                                                                              \
        template <typename Parent> static constexpr fieldType                                 \
        Parent::*fieldPtr = &Parent::Name;                                                    \
    };                                                                                        \
static_assert(true, "Forcing ;")

#define NAMED_FIELD_OPTIONAL(Type, Name, DefaultValue)      \
    Type Name = DefaultValue;                               \
    static constexpr bool Name##_optional = true;           \
    NAMED_FIELD_SET(Type, Name);                            \
static_assert(true, "Forcing ;")

#define NAMED_FIELD(Type, Name)                         \
    Type Name{};                                        \
    static constexpr bool Name##_optional = false;      \
    NAMED_FIELD_SET(Type, Name);                        \
static_assert(true, "Forcing ;")

#define DECLARE_ENTRY_TO_JSON(Self)   auto to_json(nlohmann::ordered_json &json, const Self &self) -> void;
#define DECLARE_ENTRY_FROM_JSON(Self) auto from_json(const nlohmann::json &json, Self &self) -> void;

#define DECLARE_ENTRY_BACKEND                                                                                   \
    public:                                                                                                     \
        static constexpr size_t numFields = mc::fieldIndex<0, void, FieldData>;                                 \
                                                                                                                \
        template<size_t i>                                                                                      \
        static constexpr auto fieldName = FieldData<i, void>::fieldName;                                        \
                                                                                                                \
        template<size_t i>                                                                                      \
        static constexpr auto fieldOptional = FieldData<i, void>::fieldOptional;                                \
                                                                                                                \
        template<size_t i>                                                                                      \
        static constexpr auto fieldCategory = FieldData<i, void>::fieldCategory;                                \
                                                                                                                \
        template<size_t i>                                                                                      \
        using fieldType = typename FieldData<i, void>::fieldType;                                               \
                                                                                                                \
        template<size_t i>                                                                                      \
        auto &getField() {                                                                                      \
            return this->*FieldData<i, void>::template fieldPtr<std::remove_reference_t<decltype(*this)> >;     \
        }                                                                                                       \
                                                                                                                \
        template<size_t i>                                                                                      \
        const auto &getField() const {                                                                          \
            return this->*FieldData<i, void>::template fieldPtr<std::remove_reference_t<decltype(*this)> >;     \
        }                                                                                                       \
static_assert(true, "Forcing ;")

#define DECLARE_WRITABLE_ENTRY_BACKEND                                                                          \
    public:                                                                                                     \
        auto save(const std::filesystem::path &filepath) const -> void;                                         \
                                                                                                                \
        [[nodiscard]]                                                                                           \
        auto load(const std::filesystem::path &filepath) -> bool;                                               \
                                                                                                                \
    DECLARE_ENTRY_BACKEND;                                                                                      \
static_assert(true, "Forcing ;")

#define DEFINE_ENTRY_SAVE(Self)                                         \
auto Self::save(const std::filesystem::path &filepath) const -> void {  \
    std::ofstream file(filepath);                                       \
    nlohmann::ordered_json json;                                        \
    to_json(json, *this);                                               \
    file << json.dump(4);                                               \
    file.close();                                                       \
}                                                                       \
static_assert(true, "Forcing ;")

#define DEFINE_ENTRY_LOAD(Self)                                     \
auto Self::load(const std::filesystem::path &filepath) -> bool {    \
    std::ifstream file(filepath);                                   \
    if (!file.is_open()) return false;                              \
    nlohmann::json json;                                            \
    file >> json;                                                   \
    file.close();                                                   \
    from_json(json, *this);                                         \
    return true;                                                    \
}                                                                   \
static_assert(true, "Forcing ;")

#define DEFINE_CONFIG_BACKEND(Self)                                                             \
    auto to_json(nlohmann::ordered_json &json, const Self &self) -> void {                      \
        mc::loop<Self::numFields>([&](auto i) {                                                 \
            nlohmann::ordered_json j = self.getField<i>();                                      \
            json[Self::template fieldCategory<i>][Self::template fieldName<i>] = j;             \
        });                                                                                     \
    }                                                                                           \
                                                                                                \
    auto from_json(const nlohmann::json &json, Self &self) -> void {                            \
        mc::loop<Self::numFields>([&](auto i) {                                                 \
            json.at(Self::template fieldCategory<i>).at(Self::template fieldName<i>)            \
                    .get_to(self.template getField<i>());                                       \
        });                                                                                     \
    }                                                                                           \
static_assert(true, "Forcing ;")

#define DEFINE_ENTRY_TO_JSON(Self)                                          \
    auto to_json(nlohmann::ordered_json &json, const Self &self) -> void {  \
        mc::loop<Self::numFields>([&](auto i) {                             \
            nlohmann::ordered_json j = self.template getField<i>();         \
            json[Self::template fieldName<i>] = j;                          \
        });                                                                 \
    }                                                                       \
static_assert(true, "Forcing ;")

#define DEFINE_ENTRY_FROM_JSON(Self)                                                            \
    auto from_json(const nlohmann::json &json, Self &self) -> void {                            \
        mc::loop<Self::numFields>([&](auto i) {                                                 \
            if (!json.contains(Self::template fieldName<i>) && Self::template fieldOptional<i>) \
                return;                                                                         \
            json.at(Self::template fieldName<i>).get_to(self.template getField<i>());           \
        });                                                                                     \
    }                                                                                           \
static_assert(true, "Forcing ;")
