#include <iostream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>
#include <iomanip>

std::vector<std::string> split(std::string line);

class visitor {
public:
    visitor() = default;
    visitor(std::map<std::string, int> index_map) : index_map_(index_map){}
    template<typename Property> void apply(std::string field, Property& prop) {
        auto index = index_map_.find(field);
        if(index == index_map_.end()) { return; }
        prop = boost::lexical_cast<Property>( elems_[index->second] );
    }
    void set_elems(std::vector<std::string> elems) { elems_ = elems; }
private:
    std::map<std::string, int> index_map_;
    std::vector<std::string> elems_;
};

template< typename T > class stream {
public:
    stream(std::string fields) {
        auto fields_array = split(fields);
        std::map<std::string, int> index_map;
        for(int i=0; i<fields_array.size(); ++i) {
            index_map[fields_array[i]] = i;
        }
        visitor_ = visitor(index_map);
    }
    T read() {
        std::string line;
        getline(std::cin, line);
        std::vector<std::string> elems = split(line);
        T p;
        visitor_.set_elems(elems);
        visit(p, visitor_);
        return p;
    };
private:
    visitor visitor_;
};

template<typename T> struct traits {
    template<typename V> static void visit(T& t, V& v);
};

template < typename T, typename V > inline void visit( T& t, V& v ) {
    traits< T >::visit( t, v );
}

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

template<> struct traits<P> {
    template<typename V> static void visit(P& p, V& v) {
        v.apply("x", p.x);
        v.apply("y", p.y);
    }
};

template<> struct traits<Q> {
    template<typename V> static void visit(Q& q, V& v) {
        v.apply("a", q.a);
        v.apply("b", q.b);
    }
};


int main(int argc, char* argv[]) {
    std::string fields = argv[1];
    // stream<P> s(fields);
    // while(std::cin) {
    //     P p = s.read();
    //     std::cerr << std::setprecision(16) << p.x << " " << p.y << std::endl;
    // }
    stream<Q> s(fields);
    while(std::cin) {
        Q q = s.read();
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
