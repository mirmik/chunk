#include <nos/trent/trent.h>

const char *nos::typestr(nos::trent_type m_type)
{
    switch (m_type)
    {
    case nos::trent_type::string:
        return "str";

    case nos::trent_type::list:
        return "list";

    case nos::trent_type::dict:
        return "dict";

    case nos::trent_type::numer:
        return "num";

    case nos::trent_type::boolean:
        return "bool";

    case nos::trent_type::nil:
        return "nil";
    }

    return "";
}

const nos::trent &nos::trent::static_nil()
{
    static const trent _nil;
    return _nil;
}

nos::trent::wrong_path::wrong_path(const nos::trent_path &path) : path(path)
{
    str = std::string("trent:wrong_path: ") + path.to_string();
}

const char *nos::trent::wrong_path::what() const noexcept
{
    return str.c_str();
}

nos::trent::wrong_type::wrong_type(const trent_path &path, type t, type rt)
    : path(path), t(t)
{
    str = std::string("trent:wrong_type:") + std::string(" path:") +
          path.to_string() + std::string(" request:") + nos::typestr(t) +
          std::string(" realtype:") + nos::typestr(rt);
}

const char *nos::trent::wrong_type::what() const noexcept
{
    return str.c_str();
}

nos::trent::wrong_index::wrong_index(const trent_path &path, type t)
    : path(path), t(t)
{
    str = std::string("trent:wrong_index: path: ") + path.to_string() +
          std::string(" index: ") + nos::typestr(t);
}

const char *nos::trent::wrong_index::what() const noexcept
{
    return str.c_str();
}

const char *nos::trent::typestr()
{
    return nos::typestr(m_type);
}

nos::trent::~trent()
{
    invalidate();
}

nos::trent::trent() : m_type(type::nil) {}

nos::trent::trent(const trent &other) : m_type(other.m_type)
{
    switch (m_type)
    {
    case trent::type::string:
        nos::constructor(&m_str, other.m_str);
        return;

    case trent::type::list:
        nos::constructor(&m_arr, other.m_arr);
        return;

    case trent::type::dict:
        nos::constructor(&m_dct, other.m_dct);
        return;

    case trent::type::numer:
        m_num = other.m_num;
        return;

    case trent::type::boolean:
        m_bool = other.m_bool;
        return;

    case trent::type::nil:
        return;
    }
}

nos::trent::trent(trent &&other) noexcept : m_type(other.m_type)
{
    switch (m_type)
    {
    case trent::type::string:
        nos::move_constructor(&m_str, std::move(other.m_str));
        return;

    case trent::type::list:
        nos::move_constructor(&m_arr, std::move(other.m_arr));
        return;

    case trent::type::dict:
        nos::move_constructor(&m_dct, std::move(other.m_dct));
        return;

    case trent::type::numer:
        m_num = other.m_num;
        return;

    case trent::type::boolean:
        m_bool = other.m_bool;
        return;

    case trent::type::nil:
        return;
    }

    other.invalidate();
}

void nos::trent::invalidate()
{
    switch (m_type)
    {
    case type::string:
        nos::destructor(&m_str);
        break;

    case type::list:
        nos::destructor(&m_arr);
        break;

    case type::dict:
        nos::destructor(&m_dct);
        break;

    default:
        break;
    }

    m_type = type::nil;
}

void nos::trent::init_sint(const int64_t &i)
{
    m_type = type::numer;
    m_num = static_cast<numer_type>(i);
}

void nos::trent::init_uint(const uint64_t &i)
{
    m_type = type::numer;
    m_num = i;
}

void nos::trent::init_flt(const long double &i)
{
    m_type = type::numer;
    m_num = i;
}

void nos::trent::init_str(const char *data, size_t size)
{
    m_type = type::string;
    nos::constructor(&m_str, data, size);
}

void nos::trent::init(trent::type t)
{
    if (m_type != trent::type::nil)
        invalidate();

    m_type = t;

    switch (m_type)
    {
    case trent::type::string:
        nos::constructor(&m_str);
        break;

    case trent::type::list:
        nos::constructor(&m_arr);
        break;

    case trent::type::dict:
        nos::constructor(&m_dct);
        break;

    default:
        break;
    }
}
nos::trent &nos::trent::operator[](int i)
{
    if (m_type != type::list)
        init(type::list);
    if (m_arr.size() <= (unsigned int)i)
        m_arr.resize(i + 1);
    return m_arr[i];
}

nos::trent &nos::trent::operator[](const char *key)
{
    if (m_type != type::dict)
        init(type::dict);
    return m_dct[std::string(key)];
}

nos::trent &nos::trent::operator[](const std::string &key)
{
    if (m_type != type::dict)
        init(type::dict);
    return m_dct[key];
}

nos::trent &nos::trent::operator[](const nos::buffer &key)
{
    if (m_type != type::dict)
        init(type::dict);
    return m_dct[std::string(key.data(), key.size())];
}

const nos::trent &nos::trent::operator[](const std::string &obj) const
{
    if (m_type != type::dict || !contains(obj))
        return static_nil();
    return at(obj);
}

const nos::trent &nos::trent::operator[](const nos::buffer &obj) const
{
    std::string key(obj.data(), obj.size());
    if (m_type != type::dict || !contains(key))
        return static_nil();
    return at(key);
}

const nos::trent &nos::trent::operator[](const char *obj) const
{
    if (m_type != type::dict || !contains(obj))
        return static_nil();
    return at(obj);
}

const nos::trent &nos::trent::operator[](int obj) const
{
    if (m_type != type::list)
        return static_nil();
    return at(obj);
}

const nos::trent &nos::trent::operator[](const trent_path &path) const
{
    const trent *tr = this;
    for (const auto &p : path)
    {
        if (p.is_string)
        {
            tr = &tr->at(p.str);
        }
        else
        {
            tr = &tr->at(p.i32);
        }
    }
    return *tr;
}

nos::trent &nos::trent::operator[](const trent_path &path)
{
    trent *tr = this;
    for (auto &p : path)
    {
        if (p.is_string)
        {
            tr = &((*tr)[p.str]);
        }
        else
        {
            tr = &((*tr)[p.i32]);
        }
    }
    return *tr;
}

const nos::trent &nos::trent::at(const trent_path &path) const
{
    const trent *tr = this;
    for (const auto &p : path)
    {
        if (p.is_string)
        {
            tr = &tr->at(p.str);
        }
        else
        {
            tr = &tr->at(p.i32);
        }
    }
    return *tr;
}

nos::trent &nos::trent::at(const trent_path &path)
{
    trent *tr = this;
    for (auto &p : path)
    {
        if (p.is_string)
        {
            tr = &tr->at(p.str);
        }
        else
        {
            tr = &tr->at(p.i32);
        }
    }
    return *tr;
}

bool nos::trent::contains(const char *key) const
{
    if (m_type != type::dict)
        return false;
    return m_dct.find(key) != m_dct.end();
}

bool nos::trent::contains(std::string key) const
{
    if (m_type != type::dict)
        return false;
    return m_dct.find(std::string(key)) != m_dct.end();
}

void nos::trent::init_from_list(const std::initializer_list<double> l)
{
    init(type::list);
    m_arr.reserve(l.size());
    for (auto &i : l)
    {
        m_arr.emplace_back(i);
    }
}

void nos::trent::init_from_list(const std::initializer_list<std::string> l)
{
    init(type::list);
    m_arr.reserve(l.size());
    for (auto &i : l)
    {
        m_arr.emplace_back(i);
    }
}

nos::trent &nos::trent::at(int i)
{
    if (m_type != type::list)
        throw std::runtime_error(std::string("trent: is not a list: ") +
                                 nos::typestr(m_type));
    if (m_arr.size() <= (unsigned int)i)
        throw std::runtime_error(std::string("trent:wrong_index: ") +
                                 std::to_string(i));
    return m_arr[i];
}

const nos::trent &nos::trent::at(int i) const
{
    if (m_type != type::list)
        throw std::runtime_error(std::string("trent: is not a list: ") +
                                 nos::typestr(m_type));
    if (m_arr.size() <= (unsigned int)i)
        throw std::runtime_error(std::string("trent:wrong_index: ") +
                                 std::to_string(i));
    return m_arr[i];
}

const nos::trent &nos::trent::at(const std::string &key) const
{
    if (m_type != type::dict)
        throw std::runtime_error(std::string("trent: is not a dict: ") +
                                 nos::typestr(m_type));
    return m_dct.at(key);
}

nos::trent &nos::trent::at(const std::string &key)
{
    if (m_type != type::dict)
        throw std::runtime_error(std::string("trent: is not a dict: ") +
                                 nos::typestr(m_type));
    return m_dct.at(key);
}

const nos::trent &nos::trent::at(const char *key) const
{
    if (m_type != type::dict)
        throw std::runtime_error(std::string("trent: is not a dict: ") +
                                 nos::typestr(m_type));
    return m_dct.at(std::string(key));
}

nos::trent &nos::trent::at(const char *key)
{
    if (m_type != type::dict)
        throw std::runtime_error(std::string("trent: is not a dict: ") +
                                 nos::typestr(m_type));
    return m_dct.at(std::string(key));
}

void nos::trent::push_back(const trent &tr)
{
    if (m_type != type::list)
        init(type::list);
    m_arr.push_back(tr);
}

const nos::trent *nos::trent::_get(const std::string &str) const
{
    if (is_dict())
    {
        auto it = m_dct.find(str);
        if (it == m_dct.end())
            return nullptr;
        return &it->second;
    }

    return nullptr;
}

const nos::trent *nos::trent::_get(const char *str) const
{
    return get(std::string(str));
}

const nos::trent *nos::trent::_get(int index) const
{
    if (is_list())
        return &m_arr[index];

    return nullptr;
}

const nos::trent *nos::trent::get(const trent_path &path) const
{
    const trent *tr = this;
    for (const auto &p : path)
    {
        if (p.is_string)
        {
            if (!tr->is_dict())
                return nullptr;
            tr = tr->_get(p.str);
        }
        else
        {
            tr = tr->_get(p.i32);
        }

        if (tr == nullptr)
            return nullptr;
    }
    return tr;
}

const nos::trent &nos::trent::get_except(const trent_path &path) const
{
    const trent *tr = get(path);
    if (tr == nullptr)
    {
        throw wrong_path(path);
    }
    return *tr;
}
nos::trent::numer_type
nos::trent::get_as_numer_ex(const trent_path &path) const
{
    const trent &tr = get_except(path);

    if (tr.m_type != trent_type::numer)
    {
        throw wrong_type(path, trent_type::numer, tr.m_type);
    }

    return tr.m_num;
}

const nos::trent::string_type &
nos::trent::get_as_string_ex(const trent_path &path) const
{
    const trent &tr = get_except(path);

    if (tr.m_type != trent_type::string)
    {
        throw wrong_type(path, trent_type::string, tr.m_type);
    }

    return tr.m_str;
}

bool nos::trent::get_as_boolean_ex(const trent_path &path) const
{
    const trent &tr = get_except(path);

    if (tr.m_type != trent_type::boolean)
    {
        throw wrong_type(path, trent_type::boolean, tr.m_type);
    }

    return tr.m_bool;
}

nos::trent::numer_type
nos::trent::get_as_numer_def(const trent_path &path, numer_type def) const
{
    const trent *tr = get(path);
    if (tr == nullptr || tr->m_type != trent_type::numer)
        return def;
    return tr->m_num;
}

const nos::trent::string_type &
nos::trent::get_as_string_def(const trent_path &path,
                              const std::string &def) const
{
    const trent *tr = get(path);
    if (tr == nullptr || tr->m_type != trent_type::string)
        return def;
    return tr->m_str;
}

bool nos::trent::get_as_boolean_def(const trent_path &path, bool def) const
{
    const trent *tr = get(path);
    if (tr == nullptr || tr->m_type != trent_type::boolean)
        return def;
    return tr->m_bool;
}

nos::trent::string_type &nos::trent::as_string()
{
    if (!is_string())
        init(type::string);
    return m_str;
}

const nos::trent::string_type &nos::trent::as_string() const
{
    if (!is_string())
        throw std::runtime_error(std::string("trent: is not a string: ") +
                                 nos::typestr(m_type));
    return m_str;
}

nos::expected<nos::trent::string_type &, nos::errstring>
nos::trent::as_string_critical()
{
    if (!is_string())
        return errstring("is't string");
    return m_str;
}

nos::expected<const nos::trent::string_type &, nos::errstring>
nos::trent::as_string_critical() const
{
    if (!is_string())
        return errstring("is't string");
    return m_str;
}

nos::trent::string_type &nos::trent::as_string_except()
{
    if (!is_string())
        throw std::runtime_error("isn't string");

    return m_str;
}

const nos::trent::string_type &nos::trent::as_string_except() const
{
    if (!is_string())
        throw std::runtime_error("isn't string");

    return m_str;
}

const nos::trent::string_type &
nos::trent::as_string_default(const string_type &def) const
{
    if (!is_string())
        return def;
    return m_str;
}

nos::trent::dict_type &nos::trent::as_dict()
{
    if (!is_dict())
        init(type::dict);
    return m_dct;
}

const nos::trent::dict_type &nos::trent::as_dict() const
{
    if (!is_dict())
        throw std::runtime_error("is't dict");
    return m_dct;
}

nos::expected<nos::trent::dict_type &, nos::errstring>
nos::trent::as_dict_critical()
{
    if (!is_dict())
        return errstring("is't list");
    return m_dct;
}

nos::expected<const nos::trent::dict_type &, nos::errstring>
nos::trent::as_dict_critical() const
{
    if (!is_dict())
        return errstring("is't list");
    return m_dct;
}

nos::trent::dict_type &nos::trent::as_dict_except()
{
    if (!is_dict())
        throw std::runtime_error("is't list");
    return m_dct;
}

const nos::trent::dict_type &nos::trent::as_dict_except() const
{
    if (!is_dict())
        throw std::runtime_error("is't list");
    return m_dct;
}

nos::trent::list_type &nos::trent::as_list()
{
    if (!is_list())
        init(type::list);
    return m_arr;
}

const nos::trent::list_type &nos::trent::as_list() const
{
    if (!is_list())
        throw std::runtime_error("is't list");
    return m_arr;
}

nos::expected<nos::trent::list_type &, nos::errstring>
nos::trent::as_list_critical()
{
    if (!is_list())
        return errstring("is't list");
    return m_arr;
}

nos::expected<const nos::trent::list_type &, nos::errstring>
nos::trent::as_list_critical() const
{
    if (!is_list())
        return errstring("is't list");
    return m_arr;
}

nos::trent::list_type &nos::trent::as_list_except()
{
    if (!is_list())
        throw std::runtime_error("is't list");
    return m_arr;
}

const nos::trent::list_type &nos::trent::as_list_except() const
{
    if (!is_list())
        throw std::runtime_error("is't list");
    return m_arr;
}

nos::trent::numer_type nos::trent::as_numer() const
{
    if (is_bool())
        return (int)m_bool;
    if (is_string())
        return std::stod(m_str);
    if (!is_numer())
        throw std::runtime_error("is't numer");
    return m_num;
}

nos::trent::integer_type nos::trent::as_integer() const
{
    if (is_bool())
        return (int)m_bool;
    if (!is_numer())
        throw std::runtime_error("is't numer");
    return m_num;
}

nos::expected<nos::trent::numer_type, nos::errstring>
nos::trent::as_numer_critical() const
{
    if (is_bool())
        return (int)m_bool;

    if (!is_numer())
        return errstring("is't numer");
    return m_num;
}

nos::trent::numer_type nos::trent::as_numer_except() const
{
    if (is_bool())
        return (int)m_bool;
    if (!is_numer())
        throw std::runtime_error("is't numer");
    return m_num;
}

nos::trent::numer_type nos::trent::as_numer_default(numer_type def) const
{
    if (is_bool())
        return (int)m_bool;
    if (!is_numer())
        return def;
    return m_num;
}

int64_t nos::trent::as_integer_default(int64_t def) const
{
    return as_numer_default(def);
}

nos::expected<int64_t, nos::errstring>
nos::trent::as_integer_critical() const
{
    if (!is_numer())
        return errstring("is't numer");
    return m_num;
}

int64_t nos::trent::as_integer_except() const
{
    if (!is_numer())
        throw std::runtime_error("is't numer");
    return m_num;
}

bool nos::trent::as_bool() const
{
    return m_bool;
}

bool nos::trent::as_bool_default(bool def) const
{
    if (!is_bool())
        return def;
    return m_bool;
}

nos::expected<bool, nos::errstring> nos::trent::as_bool_critical() const
{
    if (!is_bool())
        return errstring("is't bool");
    return m_bool;
}

bool nos::trent::as_bool_except() const
{
    if (!is_bool())
        throw std::runtime_error("is't bool");
    return m_bool;
}

const nos::buffer nos::trent::as_buffer() const
{
    if (!is_string())
        throw std::runtime_error("is't string");
    return nos::buffer(m_str.data(), m_str.size());
}

nos::trent::numer_type &nos::trent::unsafe_numer_const()
{
    return m_num;
}

nos::trent::string_type &nos::trent::unsafe_string_const()
{
    return m_str;
}

nos::trent::list_type &nos::trent::unsafe_list_const()
{
    return m_arr;
}

nos::trent::dict_type &nos::trent::unsafe_dict_const()
{
    return m_dct;
}

const nos::trent::numer_type &nos::trent::unsafe_numer_const() const
{
    return m_num;
}

const nos::trent::string_type &nos::trent::unsafe_string_const() const
{
    return m_str;
}

const nos::trent::list_type &nos::trent::unsafe_list_const() const
{
    return m_arr;
}

const nos::trent::dict_type &nos::trent::unsafe_dict_const() const
{
    return m_dct;
}

const bool &nos::trent::unsafe_bool_const() const
{
    return m_bool;
}

nos::trent::type nos::trent::get_type() const
{
    return m_type;
}

bool nos::trent::is_nil() const
{
    return m_type == type::nil;
}

bool nos::trent::is_bool() const
{
    return m_type == type::boolean;
}

bool nos::trent::is_numer() const
{
    return m_type == type::numer;
}

bool nos::trent::is_list() const
{
    return m_type == type::list;
}

bool nos::trent::is_dict() const
{
    return m_type == type::dict;
}

bool nos::trent::is_string() const
{
    return m_type == type::string;
}
nos::trent &nos::trent::operator=(const trent &other)
{
    if (this == &other)
        return *this;

    invalidate();
    m_type = other.m_type;
    switch (m_type)
    {
    case type::string:
        nos::constructor(&m_str, other.m_str);
        return *this;
    case type::list:
        nos::constructor(&m_arr, other.m_arr);
        return *this;
    case type::dict:
        nos::constructor(&m_dct, other.m_dct);
        return *this;
    case type::numer:
        m_num = other.m_num;
        return *this;
    case type::boolean:
        m_bool = other.m_bool;
        return *this;
    case type::nil:
        return *this;
    }
    return *this;
}

nos::trent &nos::trent::operator=(trent &&other) noexcept
{
    if (this == &other)
        return *this;

    invalidate();
    m_type = other.m_type;
    switch (m_type)
    {
    case type::string:
        nos::move_constructor(&m_str, std::move(other.m_str));
        return *this;
    case type::list:
        nos::move_constructor(&m_arr, std::move(other.m_arr));
        return *this;
    case type::dict:
        nos::move_constructor(&m_dct, std::move(other.m_dct));
        return *this;
    case type::numer:
        m_num = other.m_num;
        return *this;
    case type::boolean:
        m_bool = other.m_bool;
        return *this;
    case type::nil:
        return *this;
    }

    other.reset(type::nil);
    return *this;
}
