#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <iomanip>

std::vector<std::string> split(const std::string& line, char delimiter);
std::string join(const std::vector<std::string>& elems, char delimiter);

namespace traits {
typedef std::unordered_map<std::string, boost::optional<size_t>> IndexMap;

IndexMap index_map_from(const std::string& fields) {
    IndexMap map;
    auto split_fields = split(fields, ',');
    for(size_t i=0; i<split_fields.size(); ++i) {
        auto field = split_fields[i];
        if(field.empty()) { continue; }
        std::string path;
        for(const auto& key: split(field, '/')) {
            path += (path.empty() ? "" : "/") + key;
            map[path];
        }
        map[path] = i;
    }
    return map;
}

struct Payload {
    Payload(const std::string& fields) : index_map(index_map_from(fields)) {}
    std::vector<std::string> values;
    const IndexMap index_map;
    mutable std::string path;
};

template<typename T> void apply(T& t, const Payload& payload) {
    auto it = payload.index_map.find(payload.path);
    if(it == payload.index_map.end()) { return; }
    auto value = payload.values[*it->second];
    t = boost::lexical_cast<T>(value);
}

template<typename T> void visit(const std::string& key, T& t, const Payload& payload) {
    auto original_path = payload.path;
    payload.path += (payload.path.empty() ? "" : "/") + key;
    if(payload.index_map.find(payload.path) != payload.index_map.end()) {
        apply(t, payload);
    }
    payload.path = original_path;
}
} // namespace traits {

template< typename T > class Stream {
public:
    Stream(const std::string& fields) : payload(fields) {}
    T read(const std::string& line) {
        T t;
        payload.values = split(line, ',');
        traits::apply(t, payload);
        return t;
    };
private:
    traits::Payload payload;
};


struct C {
    bool f;
    C(): f(true) {}
};

struct B {
    double x;
    int i;
    C c;
    B(): x(-2.1), i(10) {}
};

struct A {
    B b;
    std::string m;
    A(): m("hello") {}
};

namespace traits {
template<> void apply(C& c, const Payload& payload) {
    visit("f", c.f, payload);
}

template<> void apply(B& b, const Payload& payload) {
    visit("x", b.x, payload);
    visit("i", b.i, payload);
    visit("c", b.c, payload);
}

template<> void apply(A& a, const Payload& payload) {
    visit("b", a.b, payload);
    visit("m", a.m, payload);
}
}


// ./a.out "b/x,b/i,b/c/f,m" < <( echo "-2.1,123,0,cats"; echo "12.23,-10,1,dogs" )
int main(int argc, char* argv[]) {
    std::string fields = argv[1];
    Stream<A> stream(fields);
    std::string line;
    while(getline(std::cin, line)) {
        A a = stream.read(line);
        std::cerr << std::setprecision(16)
            << " b/x=" << a.b.x
            << " b/i=" << a.b.i
            << " b/c/f=" << a.b.c.f
            << " m=" << a.m << std::endl;
    }
}

std::vector<std::string> split(const std::string& line, char delimiter) {
    std::vector<std::string> elems;
    std::stringstream ss(line);
    std::string elem;
    while(std::getline(ss, elem, delimiter)) {
        elems.push_back(elem);
    }
    return elems;
}

std::string join(const std::vector<std::string>& elems, char delimiter) {
    if(elems.empty()) { return ""; }
    if(elems.size() == 1) { return elems[0]; }
    std::stringstream ss;
    ss << elems[0];
    for(size_t i=1; i<elems.size(); ++i) {
        ss << delimiter << elems[i];
    }
    return ss.str();
}
