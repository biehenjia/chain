#include <chains/print.hpp>
#include <chains/node.hpp>
#include <symengine/printers.h>
#include <sstream>
#include <string>

namespace chains {

    namespace {

        const char* kind_name(Kind k) {
            switch (k) {
                case Kind::Leaf: return "Leaf";
                case Kind::Sum: return "Sum";
                case Kind::Prod: return "Prod";
                case Kind::Sin: return "Sin";
                case Kind::Cos: return "Cos";
                case Kind::Tan: return "Tan";
                case Kind::Cot: return "Cot";
                case Kind::EAdd: return "EAdd";
                case Kind::EMul: return "EMul";
                case Kind::EPow: return "EPow";
                case Kind::ELog: return "ELog";
                case Kind::ESin: return "ESin";
                case Kind::ECos: return "ECos";
                case Kind::ETan: return "ETan";
                case Kind::ECot: return "ECot";
                case Kind::Connector: return "Connector";
            }
            return "?";
        }

        void rec(const Arena& a, uint32_t id, std::ostringstream& os) {
            const Node& n = a.node(id);
            switch (n.kind) {
                case Kind::Leaf: {
                    os << SymEngine::str(*a.leaf_symbolic(id));
                    return;
                }
                case Kind::Connector: {
                    os << '@' << n.slot_a << ':' << n.slot_b;
                    return;
                }
                case Kind::Sum:
                case Kind::Prod: {
                    auto ops = a.operands(id);
                    os << kind_name(n.kind) << '_' << unsigned(n.variable) << '[';
                    for (size_t i = 0; i < ops.size(); ++i) {
                        if (i) os << ", ";
                        rec(a, ops[i], os);
                    }
                    os << ']';
                    return;
                }
                case Kind::Sin:
                case Kind::Cos:
                case Kind::Tan:
                case Kind::Cot: {
                    auto ops = a.operands(id);
                    size_t hl = ops.size() / 2;
                    os << kind_name(n.kind) << '_' << unsigned(n.variable) << '[';
                    for (size_t i = 0; i < hl; ++i) {
                        if (i) os << ", ";
                        rec(a, ops[i], os);
                    }
                    os << " | ";
                    for (size_t i = hl; i < ops.size(); ++i) {
                        if (i != hl) os << ", ";
                        rec(a, ops[i], os);
                    }
                    os << ']';
                    return;
                }
                case Kind::EAdd:
                case Kind::EMul:
                case Kind::EPow:
                case Kind::ELog:
                case Kind::ESin:
                case Kind::ECos:
                case Kind::ETan:
                case Kind::ECot: {
                    auto ops = a.operands(id);
                    os << kind_name(n.kind) << '(';
                    for (size_t i = 0; i < ops.size(); ++i) {
                        if (i) os << ", ";
                        rec(a, ops[i], os);
                    }
                    os << ')';
                    return;
                }
            }
        }

    }

    std::string to_string(const Arena& a, uint32_t id) {
        std::ostringstream os;
        rec(a, id, os);
        return os.str();
    }

}
