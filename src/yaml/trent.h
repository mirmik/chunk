#ifndef nos_TRENT_TRENT_H
#define nos_TRENT_TRENT_H

/**
    @file
*/

#include <cassert>
#include <cstdint>
#include <cstring>
#include <initializer_list>
#include <map>
#include <nos/buffer.h>
#include <nos/print.h>
#include <nos/trent/trent_path.h>
#include <nos/util/ctrdtr.h>
#include <nos/util/error.h>
#include <nos/util/expected.h>
#include <nos/util/flat_map.h>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace nos
{
    enum class trent_type : uint8_t
    {
        string,
        list,
        dict,
        numer,
        boolean,
        nil,
    };
    const char *typestr(trent_type type);

    class trent_path;

    class trent
    {
    public:
        static const trent &static_nil();
        static trent nil()
        {
            return trent();
        }

        using type = trent_type;
        using value_type = std::pair<std::string, trent>;

        using list_type = std::vector<trent>;
        using dict_type = nos::flat_map<std::string, trent>;
        using string_type = std::string;
        using numer_type = long double;
        using integer_type = int64_t;

        class wrong_path : public std::exception
        {
            trent_path path = {};
            std::string str = {};

        public:
            explicit wrong_path(const nos::trent_path &path);
            const char *what() const noexcept override;
        };

        class wrong_type : public std::exception
        {
        public:
            trent_path path = {};
            type t = {};
            std::string str = {};

        public:
            wrong_type(const trent_path &path, type t, type rt);
            const char *what() const noexcept override;
        };

        class wrong_index : public std::exception
        {
        public:
            std::string str;
            trent_path path;
            type t;

        public:
            wrong_index(const trent_path &path, type t);
            const char *what() const noexcept override;
        };

    private:
        type m_type = type::nil;
        union
        {
            bool m_bool;
            numer_type m_num;
            list_type m_arr;
            dict_type m_dct;
            string_type m_str;
        };

    public:
        const char *typestr();

        ~trent();
        trent();
        trent(const trent &other);
        trent(trent &&other) noexcept;

        void invalidate();

        template <class T> trent(const T &obj)
        {
            init(obj);
        }

        void init_sint(const int64_t &i);
        void init_uint(const uint64_t &i);
        void init_flt(const long double &i);
        void init_str(const char *data, size_t size);

        void init(type t);

        void init(const char *ptr)
        {
            init_str(ptr, strlen(ptr));
        }
        void init(const std::string &str)
        {
            init_str(str.data(), str.size());
        }
        void init(const nos::buffer &str)
        {
            init_str(str.data(), str.size());
        }

        void init(const int8_t &i)
        {
            init_sint(i);
        }
        void init(const int16_t &i)
        {
            init_sint(i);
        }
        void init(const int32_t &i)
        {
            init_sint(i);
        }
        void init(const int64_t &i)
        {
            init_sint(i);
        }

        void init(const uint8_t &i)
        {
            init_uint(i);
        }
        void init(const uint16_t &i)
        {
            init_uint(i);
        }
        void init(const uint32_t &i)
        {
            init_uint(i);
        }
        void init(const uint64_t &i)
        {
            init_uint(i);
        }

        void init(const float &i)
        {
            init_flt(i);
        }
        void init(const double &i)
        {
            init_flt(i);
        }
        void init(const long double &i)
        {
            init_flt(i);
        }

        void init(const bool &i)
        {
            m_type = type::boolean;
            m_bool = i;
        }

        template <typename T> void reset(const T &obj)
        {
            invalidate();
            init(obj);
        }

        trent &operator[](int i);
        trent &operator[](const char *key);
        trent &operator[](const std::string &key);
        trent &operator[](const nos::buffer &key);
        trent &operator[](const trent_path &path);

        const trent &operator[](const std::string &obj) const;
        const trent &operator[](const nos::buffer &obj) const;
        const trent &operator[](const char *obj) const;
        const trent &operator[](int obj) const;
        const trent &operator[](const trent_path &path) const;

        const trent &at(const trent_path &path) const;
        trent &at(const trent_path &path);

        bool contains(const char *key) const;
        bool contains(std::string key) const;

        void init_from_list(const std::initializer_list<double> l);
        void init_from_list(const std::initializer_list<std::string> l);

        trent &at(int i);
        const trent &at(int i) const;
        trent &at(const std::string &key);
        const trent &at(const std::string &key) const;
        trent &at(const char *key);
        const trent &at(const char *key) const;

        void push_back(const trent &tr);

        const trent *_get(const std::string &str) const;
        const trent *_get(const char *str) const;
        const trent *_get(int index) const;
        const trent *get(const trent_path &path) const;
        const trent &get_except(const trent_path &path) const;

        numer_type get_as_numer_ex(const trent_path &path) const;
        const string_type &get_as_string_ex(const trent_path &path) const;
        bool get_as_boolean_ex(const trent_path &path) const;

        numer_type get_as_numer_def(const trent_path &path,
                                    numer_type def) const;
        const string_type &get_as_string_def(const trent_path &path,
                                             const std::string &def) const;
        bool get_as_boolean_def(const trent_path &path, bool def) const;

        string_type &as_string();
        const string_type &as_string() const;
        nos::expected<string_type &, nos::errstring> as_string_critical();
        nos::expected<const string_type &, nos::errstring>
        as_string_critical() const;
        string_type &as_string_except();
        const string_type &as_string_except() const;
        const string_type &as_string_default(const string_type &def) const;

        dict_type &as_dict();
        const dict_type &as_dict() const;
        nos::expected<dict_type &, nos::errstring> as_dict_critical();
        nos::expected<const dict_type &, nos::errstring>
        as_dict_critical() const;
        dict_type &as_dict_except();
        const dict_type &as_dict_except() const;

        list_type &as_list();
        const list_type &as_list() const;
        nos::expected<list_type &, nos::errstring> as_list_critical();
        nos::expected<const list_type &, nos::errstring>
        as_list_critical() const;
        list_type &as_list_except();
        const list_type &as_list_except() const;

        numer_type as_numer() const;
        integer_type as_integer() const;
        nos::expected<numer_type, nos::errstring> as_numer_critical() const;
        numer_type as_numer_except() const;
        numer_type as_numer_default(numer_type def) const;
        int64_t as_integer_default(int64_t def) const;
        nos::expected<int64_t, nos::errstring> as_integer_critical() const;
        int64_t as_integer_except() const;

        bool as_bool() const;
        bool as_bool_default(bool def) const;
        nos::expected<bool, nos::errstring> as_bool_critical() const;
        bool as_bool_except() const;

        const nos::buffer as_buffer() const;

        numer_type &unsafe_numer_const();
        string_type &unsafe_string_const();
        list_type &unsafe_list_const();
        dict_type &unsafe_dict_const();

        const numer_type &unsafe_numer_const() const;
        const string_type &unsafe_string_const() const;
        const list_type &unsafe_list_const() const;
        const dict_type &unsafe_dict_const() const;
        const bool &unsafe_bool_const() const;

        trent::type get_type() const;
        bool is_nil() const;
        bool is_bool() const;
        bool is_numer() const;
        bool is_list() const;
        bool is_dict() const;
        bool is_string() const;

        template <class O>
        nos::expected<size_t, nos::output_error> print_to(O &os) const;

        trent &operator=(const trent &other);
        trent &operator=(trent &&other) noexcept;

        template <class T> trent &operator=(const T &arg)
        {
            reset(arg);
            return *this;
        }

        template <class T> T get() const
        {
            using DT = typename std::decay<T>::type;

            if constexpr (std::is_same_v<DT, trent>)
            {
                return *this;
            }
            else if constexpr (std::is_same_v<DT, std::string>)
            {
                if (is_string())
                    return as_string();
                else
                    return std::to_string(as_numer_except());
            }
            else if constexpr (std::is_same_v<DT, bool>)
            {
                return as_bool();
            }
            else
            {
                if (is_numer())
                    return static_cast<T>(as_numer());
                else
                    return static_cast<T>(std::stod(as_string()));
            }
        }
    };

    template <class O>
    nos::expected<size_t, nos::output_error> trent::print_to(O &os) const
    {
        bool sep = false;

        switch (get_type())
        {
        case type::boolean:
            nos::print_to(os, unsafe_bool_const() ? "true" : "false");
            return 0;

        case type::numer:
            nos::print_to(os, unsafe_numer_const());
            return 0;

        case type::string:
            os.putbyte('"');
            nos::print_to(os, unsafe_string_const());
            os.putbyte('"');
            return 0;

        case type::list:
            os.putbyte('[');

            for (auto &v : unsafe_list_const())
            {
                if (sep)
                    os.putbyte(',');

                v.print_to(os);
                sep = true;
            }

            os.putbyte(']');
            return 0;

        case type::dict:
            os.putbyte('{');

            for (auto &p : unsafe_dict_const())
            {
                if (sep)
                    os.putbyte(',');

                os.putbyte('"');
                nos::print_to(os, p.first);
                os.putbyte('"');
                os.putbyte(':');
                p.second.print_to(os);
                sep = true;
            }

            os.putbyte('}');
            return 0;

        case type::nil:
            nos::print_to(os, "nil");
            return 0;
        }
        return 0;
    }

} // namespace nos

#endif
