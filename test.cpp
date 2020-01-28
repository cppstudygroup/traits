#include <iostream>
#include <boost/lexical_cast.hpp>
#include <vector>
#include <map>

std::vector<std::string> split(std::string line);

template< typename T > class stream {
public:
    stream(std::string fields) : fields(fields) {
        fields_array = split(fields);
        for(int i=0; i<fields_array.size(); ++i) {
            if (fields_array[i]=="x") { fields_map["x"] = i; }
            if (fields_array[i]=="y") { fields_map["y"] = i; }
        }
    }
    T read() {
        std::string line;
        getline(std::cin, line);
        std::vector<std::string> elems = split(line);
        result_ = default_;
        T p;
        lexical_cast_( p.x, elems[fields_map["x"]] );
        lexical_cast_( p.y, elems[fields_map["y"]] );
        return p;
    };
private:
    std::string fields;
    std::vector<std::string> fields_array;
    std::map<std::string, int> fields_map;
    T default_ = T();
    T result_;

    static void lexical_cast_( int& v, const std::string& s ) {
        v = boost::lexical_cast< int >( s );
    }
    static void lexical_cast_( std::string& v, const std::string& s ) {
        v = s;
    }
    // todo: add more for other types
};

template<typename T> struct traits {
    template<typename V> static void visit(T& t, V& v);
};

template < typename T, typename V > inline void visit( T& t, V& v ) {
    traits< T >::visit( t, v );
}

struct P {
    int x;
    int y;
    P(): x(0), y(0) {}
};

struct Q {
    std::string a;
    bool b;
    P p;
    Q(): b(true), p() {}
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
        v.apply("p", q.p);
    }
};


std::string fields = "cat,x,,dog,y,,"; // fields for P
//std::string fields = "a,p/x,b,p/y"; // fields for Q

int main() {
    stream<P> s(fields);
    while(std::cin) {
        P p = s.read();
        std::cerr << p.x << " " << p.y << std::endl;
    }
    // stream<Q> s(fields);
    // while(std::cin) {
    //     Q q = s.read();
    //     std::cerr << q.a << " " << q.b  << " " << q.p.x << " " << q.p.y << std::endl;
    // }
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
