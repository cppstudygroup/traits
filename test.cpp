#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <type_traits>

std::vector<std::string> split(std::string line, char delimiter);
std::string join(std::vector<std::string> elems, char delimiter);

namespace visiting {
typedef std::unordered_map<std::string, size_t> IndexMap;

IndexMap index_map_from(std::string fields) {
    IndexMap map;
    auto split_fields = split(fields, ',');
    for(size_t i=0; i<split_fields.size(); ++i) {
        map[split_fields[i]] = i;
    }
    return map;
}

struct Payload {
    Payload(std::string fields) : index_map(index_map_from(fields)) {}
    std::vector<std::string> path;
    std::vector<std::string> values;
    const IndexMap index_map;
};

template<typename T> void apply(std::string key, T& t, Payload payload) {
    if(!key.empty()) { payload.path.push_back(key); }
    auto path = join(payload.path, '/');
    auto it = payload.index_map.find(path);
    if(it == payload.index_map.end()) { return; }
    auto value = payload.values[it->second];
    t = boost::lexical_cast<T>(value);
}
}

template< typename T > class Stream {
public:
    Stream(std::string fields) : payload(fields) {}
    T read(std::string line) {
        T t;
        payload.values = split(line, ',');
        visiting::apply("", t, payload);
        return t;
    };
private:
    visiting::Payload payload;
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

namespace visiting {
template<> void apply(std::string key, C& c, Payload payload) {
    if(!key.empty()) { payload.path.push_back(key); }
    apply("f", c.f, payload);
}

template<> void apply(std::string key, B& b, Payload payload) {
    if(!key.empty()) { payload.path.push_back(key); }
    apply("x", b.x, payload);
    apply("i", b.i, payload);
    apply("c", b.c, payload);
}

template<> void apply(std::string key, A& a, Payload payload) {
    if(!key.empty()) { payload.path.push_back(key); }
    apply("b", a.b, payload);
    apply("m", a.m, payload);
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

std::vector<std::string> split(std::string line, char delimiter) {
    std::vector<std::string> elems;
    std::stringstream ss(line);
    std::string elem;
    while(std::getline(ss, elem, delimiter)) {
        elems.push_back(elem);
    }
    return elems;
}

std::string join(std::vector<std::string> elems, char delimiter) {
    if(elems.empty()) { return ""; }
    if(elems.size() == 1) { return elems[0]; }
    std::stringstream ss;
    ss << elems[0];
    for(size_t i=1; i<elems.size(); ++i) {
        ss << delimiter << elems[i];
    }
    return ss.str();
}
