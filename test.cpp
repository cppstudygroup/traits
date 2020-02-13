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

IndexMap index_map_from(const std::vector<std::string>& fields) {
    IndexMap map;
    for(size_t i=0; i<fields.size(); ++i) {
        if(fields[i].empty()) { continue; }
        std::string path;
        for(const auto& key: split(fields[i], '/')) {
            path += (path.empty() ? "" : "/") + key;
            map[path];
        }
        map[path] = i;
    }
    return map;
}

struct Payload {
    Payload(const std::vector<std::string>& fields) : index_map(index_map_from(fields)) {}
    std::vector<std::string> values;
    const IndexMap index_map;
};

template<typename T> void apply(T& t, const Payload& payload, std::string path) {
    auto it = payload.index_map.find(path);
    if(it == payload.index_map.end()) { return; }
    auto value = payload.values[*it->second];
    t = boost::lexical_cast<T>(value);
}

template<typename T> void visit(const std::string& key, T& t, const Payload& payload, std::string path) {
    path += (path.empty() ? "" : "/") + key;
    if(payload.index_map.find(path) == payload.index_map.end()) { return; }
    apply(t, payload, path);
}
} // namespace traits {

template< typename T > class Stream {
public:
    Stream(std::istream& istream, const std::string& fields)
        : istream(istream)
        , payload(split(fields, ','))
        {}
    boost::optional<T> read() {
        T t;
        std::string line;
        getline(istream, line, '\n');
        if(line.empty()) { return boost::none; }
        payload.values = split(line, ',');
        traits::apply(t, payload, "");
        return t;
    };
private:
    std::istream& istream;
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
template<> void apply(C& c, const Payload& payload, std::string path) {
    visit("f", c.f, payload, path);
}

template<> void apply(B& b, const Payload& payload, std::string path) {
    visit("x", b.x, payload, path);
    visit("i", b.i, payload, path);
    visit("c", b.c, payload, path);
}

template<> void apply(A& a, const Payload& payload, std::string path) {
    visit("b", a.b, payload, path);
    visit("m", a.m, payload, path);
}
}


// ./a.out "b/x,b/i,b/c/f,m" < <( echo "-2.1,123,0,cats"; echo "12.23,-10,1,dogs" )
int main(int argc, char* argv[]) {
    std::string fields = argv[1];
    Stream<A> stream(std::cin, fields);
    std::string line;
    while(std::cin) {
        boost::optional<A> a = stream.read();
        if(!a) { break; }
        std::cerr << std::setprecision(16)
            << " b/x=" << a->b.x
            << " b/i=" << a->b.i
            << " b/c/f=" << a->b.c.f
            << " m=" << a->m << std::endl;
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
