#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <iostream>
#include <vector>
#include <map>
#include <set>
#include <iomanip>
#include <type_traits>

std::vector<std::string> split(std::string line, char delimiter);
std::string join(std::vector<std::string> elems, char delimiter);

template<typename T> struct Traits {
    template<typename Visitor> static void visit(T& t, const Visitor& visitor);
};

class Visitor {
public:
    typedef std::pair<std::string, std::string> Path;

    Visitor() = default;
    Visitor(size_t index) : index_(index) {}
    Visitor(std::string fields) {
        std::set<std::string> roots;
        std::vector<Path> paths;
        auto split_fields = split(fields, ',');
        for(size_t i=0; i<split_fields.size(); ++i) {
            if(split_fields[i].empty()) { continue; }
            auto pos = split_fields[i].find_first_of('/');
            if(pos == std::string::npos) {
                paths.push_back(std::make_pair(split_fields[i], ""));
                visitor_map_[split_fields[i]] = Visitor(i);
                continue;
            }
            auto root = split_fields[i].substr(0, pos);
            auto rest = split_fields[i].substr(pos+1);
            roots.insert(root);
            paths.push_back(std::make_pair(root, rest));
        }

        for(std::string root: roots) {
            std::vector<std::string> subfields;
            for(auto path: paths) {
                auto subpath = path.first != root ? "" : path.second;
                subfields.push_back(subpath);
            }
            visitor_map_[root] = Visitor(join(subfields, ','));
        }
    }

    template<typename Property> void apply(std::string field, Property& property) const {
        auto it = visitor_map_.find(field);
        if(it == visitor_map_.end()) { return; }
        if constexpr(std::is_arithmetic<Property>::value || std::is_same<Property,std::string>::value) {
            property = boost::lexical_cast<Property>(elems_[*it->second.index_]);
        } else {
            Traits<Property>::visit(property, it->second);
        }
    }

    void set_elems(std::string line) {
        elems_ = split(line, ',');
        for(auto& kv: visitor_map_) {
            kv.second.set_elems(line);
        }
    }

    void show() const {
        std::cerr << (index_ ? *index_ : -1) << std::endl;
        for(auto kv: visitor_map_) {
            std::cerr << "key: " << kv.first << std::endl;
            kv.second.show();
        }
    }

private:
    std::map<std::string, Visitor> visitor_map_;
    std::optional<size_t> index_;
    std::vector<std::string> elems_;
};

template< typename T > class Stream {
public:
    Stream(std::string fields) : visitor_(fields) { visitor_.show(); }
    T read() {
        std::string line;
        getline(std::cin, line);
        T t;
        visitor_.set_elems(line);
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
