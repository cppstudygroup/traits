#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <iomanip>
#include <type_traits>

std::vector<std::string> split(std::string line);
template<typename T> struct Traits;

class Visitor {
public:
    Visitor() = default;
    Visitor(std::map<std::string, unsigned int> index_map) : index_map_(index_map) {}
    template<typename Property> void apply(std::string field, Property& property) const {
        if constexpr(std::is_arithmetic<Property>::value || std::is_same<Property,std::string>::value) {
            auto index = index_map_.find(field);
            if(index == index_map_.end()) { return; }
            property = boost::lexical_cast<Property>(elems_[index->second]);
        } else {
            std::string prefix = field + "/";
            std::map<std::string, unsigned int> index_map;
            for(auto kv: index_map_) {
                if(!boost::starts_with(kv.first, prefix)) { continue; }
                auto field = kv.first;
                auto index = kv.second;
                boost::algorithm::erase_first(field, prefix);
                index_map[field] = index;
            }
            if(index_map.empty()) { return; }
            auto visitor = Visitor(index_map);
            visitor.set_elems(elems_);
            Traits<Property>::visit(property, visitor);
        }
    }
    void set_elems(std::vector<std::string> elems) { elems_ = elems; }
private:
    std::map<std::string, unsigned int> index_map_;
    std::vector<std::string> elems_;
};

template<typename T> struct Traits {
    static void visit(T& t, const Visitor& visitor);
};

template< typename T > class Stream {
public:
    Stream(std::string fields) {
        auto fields_array = split(fields);
        std::map<std::string, unsigned int> index_map;
        for(int i=0; i<fields_array.size(); ++i) {
            index_map[fields_array[i]] = i;
        }
        visitor_ = Visitor(index_map);
    }
    T read() {
        std::string line;
        getline(std::cin, line);
        std::vector<std::string> elems = split(line);
        T t;
        visitor_.set_elems(elems);
        Traits<T>::visit(t, visitor_);
        return t;
    };
private:
    Visitor visitor_;
};


struct P {
    double x;
    int y;
    P(): x(-2.1), y(10) {}
};

struct Q {
    std::string a;
    bool b;
    P p;
    Q(): a("hello"), b(true) {}
};

template<> struct Traits<P> {
    static void visit(P& p, const Visitor& visitor) {
        visitor.apply("x", p.x);
        visitor.apply("y", p.y);
    }
};

template<> struct Traits<Q> {
    static void visit(Q& q, const Visitor& visitor) {
        visitor.apply("a", q.a);
        visitor.apply("b", q.b);
        visitor.apply("p", q.p);
    }
};

// fields "a,b,p/x,p/y"
int main(int argc, char* argv[]) {
    std::string fields = argv[1];
    Stream<Q> stream(fields);
    while(std::cin) {
        Q q = stream.read();
        std::cerr << std::setprecision(16)
            << " q.a=" << q.a << " q.b=" << q.b
            << " q.p.x=" << q.p.x << " q.p.y=" << q.p.y << std::endl;
    }
}

std::vector<std::string> split(std::string line) {
    std::vector<std::string> elems;
    std::stringstream ss(line);
    std::string elem;
    while (std::getline(ss, elem, ',')) {
        elems.push_back(elem);
    }
    return elems;
}
