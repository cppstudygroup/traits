#include <iostream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>
#include <iomanip>

std::vector<std::string> split(std::string line);

class Visitor {
public:
    Visitor() = default;
    Visitor(std::map<std::string, unsigned int> index_map) : index_map_(index_map){}
    template<typename Property> void apply(std::string field, Property& property) const {
        auto index = index_map_.find(field);
        if(index == index_map_.end()) { return; }
        property = boost::lexical_cast<Property>(elems_[index->second]);
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
    }
};


int main(int argc, char* argv[]) {
    std::string fields = argv[1];
    // stream<P> stream(fields);
    // while(std::cin) {
    //     P p = stream.read();
    //     std::cerr << std::setprecision(16) << p.x << " " << p.y << std::endl;
    // }
    Stream<Q> stream(fields);
    while(std::cin) {
        Q q = stream.read();
        std::cerr << q.a << " " << q.b << std::endl;
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
