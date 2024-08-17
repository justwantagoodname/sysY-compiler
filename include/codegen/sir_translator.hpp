#pragma once
#ifndef SIR_TRANSLATOR
#define SIR_TRANSLATOR

#include "ast.h"
#include "sir.hpp"

namespace SIRTranslatorGroup {

    using namespace SIR;

    class SIRTranslator {
        NodeRef comp_unit;

        std::shared_ptr<Function> cur_function;
        std::shared_ptr<Unit> result;

        static void
        passType(NodeRef cur, const NodeRef child, const char *to_attr = "type", const char *from_attr = "type") {
            string type;
            ASTNode_get_attr_str_s(child, from_attr, type);
            ASTNode_set_attr_str(cur, to_attr, type);
        }

        static SyType getType(NodeRef node) {
            assert(node);

            string ret;
            ASTNode_get_attr_str_s(node, "type", ret);
            return ret;
        }

        static void setType(NodeRef node, const SyType &cur) {
            ASTNode_set_attr_str(node, "type", cur);
        }

    public:
        SIRTranslator(NodeRef comp_unit) {
            this->comp_unit = comp_unit;
            result = std::make_shared<Unit>();
        }

        auto translate() {
            auto funcs = ASTNode_querySelector(this->comp_unit, "/Scope/FunctionDef/Function");

            funcs->foreach([&](auto func) {
                cur_function = make_shared<Function>(func);
                assert(!(result->hasFunction(cur_function->name)));
                result->functions.insert({cur_function->name, cur_function});
                translateFunc(func);
            });

            return this->result;
        }

        void translateFunc(NodeRef func) {
            assert(ASTNode_id_is(func, "Function"));

            int i = 0;
            // 查找所有的参数，为参数生成引用 local slot
            $(func, "/Scope/Decl/ParamDecl")->foreach([&](auto param) {
                ASTNode_set_attr_int(param, "index", i++);
            });

            translateBlock(*$(func, "/Scope/Block"));
        }

        void translateBlock(NodeRef block) {
            assert(ASTNode_id_is(block, "Block"));

            $(block, "*")->foreach([&](auto stmt) {
                translateStmt(stmt);
            });
        }

        void translateStmt(NodeRef stmt) {
            When(stmt, {
                    TagMatch<void>("NOP", [&]() {
                        cur_function->add_instr(I::nop());
                    }),
                    TagMatch<void>("Exp", [&]() {
                        translateExp(stmt);
                    }),
            });
        }

        void translateExp(NodeRef exp) {
            assert(ASTNode_id_is(exp, "Exp"));
            auto inner = *$(exp, "*");
            translateExpInner(inner);
            passType(exp, inner);
        }

        void translateNumber(NodeRef number) {
            assert(ASTNode_id_is(number, "Number"));
            When(number, {
                    TypeIs<void>(SyInt, [&]() {
                        int value;
                        ASTNode_get_attr_int_s(number, "type", &value);
                        cur_function->add_instr(I::iconst(value));
                    }),
                    TypeIs<void>(SyFloat, [&]() {
                        float value;
                        ASTNode_get_attr_float_s(number, "type", &value);
                        cur_function->add_instr(I::fconst(value));
                    })
            });
        }

        void translateExpInner(NodeRef exp) {
            When(exp, {
                    TagMatch<void>("Number", [&]() {
                        translateNumber(exp);
                    }),
                    TagMatch<void>("Call", [&]() {
                        translateCall(exp);
                    }),
                    TagMatch<void>("Fetch", [&]() {
                        translateFetch(exp);
                    }),
                    TagMatch<void>({"Plus", "Minus", "Mult", "Div", "Mod"}, [&]() {
                        translateArithmetic(exp);
                    }),
                    TagMatch<void>({"UnPlus", "UnMinus", "Not"}, [&]() {
                        translateUnary(exp);
                    }),
                    TagMatch<void>({"Equal", "NotEq", "Less", "LessEq", "Greater", "GreaterEq"}, [&]() {
                        translateRelative(exp);
                    }),
                    TagMatch<void>({"Or", "And"}, [&]() {
                        translateLogic(exp);
                    })
            });
        }

        void translateCall(NodeRef call) {
            // TODO
        }

        void translateFetch(NodeRef fetch) {
            NodeRef address = *$(fetch, "Address");
            assert(address);
            // TODO: Need fix
            panic("TODO");
        }

        void translateArithmetic(NodeRef exp) {
            assert(ASTNode_id_is(exp, "Plus")
                   || ASTNode_id_is(exp, "Minus")
                   || ASTNode_id_is(exp, "Mult")
                   || ASTNode_id_is(exp, "Div")
                   || ASTNode_id_is(exp, "Mod"));

            NodeRef lhs = *$(exp, "*[0]"),
                    rhs = *$(exp, "*[1]");
            assert(lhs != nullptr && rhs != nullptr);

            SyType lhs_type, rhs_type;
            translateExpInner(lhs);
            lhs_type = getType(lhs);
            auto lhs_type_convert_pos = cur_function->get_current_pos();

            translateExpInner(rhs);
            rhs_type = getType(rhs);

            assert(lhs_type == SyFloat || lhs_type == SyInt);
            assert(rhs_type == SyFloat || rhs_type == SyInt);

            if (rhs_type != lhs_type) {
                if (lhs_type == SyInt) {
                    cur_function->add_instr_at(lhs_type_convert_pos, I::i2f());
                    lhs_type = rhs_type;
                } else if (rhs_type == SyInt) {
                    cur_function->add_instr(I::i2f());
                    rhs_type = lhs_type;
                }
            }

            assert(lhs_type == rhs_type);
            setType(exp, lhs_type);
            auto instr = When(exp, {
                    TagMatch<InstructionRef>("Plus", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::iadd();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fadd();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Minus", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::isub();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fsub();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Mult", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::imul();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fmul();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Div", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::idiv();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fdiv();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Mod", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::irem();
                                })
                        });
                    })
            });

            cur_function->add_instr(instr);
        }

        void translateUnary(NodeRef exp) {

            auto inner = *$(exp, "*");
            translateExpInner(inner);
            passType(exp, inner);

            // *NOTE*: Look *inner* type to decide which instr to use.
            auto instr = When(inner, {
                    TagMatch<InstructionRef>("UnPlus", [&]() {
                        return I::nop();
                    }),
                    TagMatch<InstructionRef>("UnMinus", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::ineg();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fneg();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Not", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::inot();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    setType(exp, SyInt); // Note the type was changed.
                                    return I::fnot();
                                }),

                        });
                    })
            });
            cur_function->add_instr(instr);
        }

        void translateRelative(NodeRef exp) {
            assert(ASTNode_id_is(exp, "Equal")
                   || ASTNode_id_is(exp, "NotEq")
                   || ASTNode_id_is(exp, "Less")
                   || ASTNode_id_is(exp, "LessEq")
                   || ASTNode_id_is(exp, "Greater")
                   || ASTNode_id_is(exp, "GreaterEq"));

            NodeRef lhs = *$(exp, "*[0]"),
                    rhs = *$(exp, "*[1]");
            assert(lhs && rhs);

            translateExpInner(lhs);
            auto lhs_type = getType(lhs);
            auto lhs_type_convert_pos = cur_function->get_current_pos();

            translateExpInner(rhs);
            auto rhs_type = getType(rhs);
            assert(lhs_type == SyInt || lhs_type == SyFloat);
            assert(rhs_type == SyInt || rhs_type == SyFloat);

            if (lhs_type != rhs_type) {
                if (lhs_type == SyInt) {
                    cur_function->add_instr_at(lhs_type_convert_pos, I::i2f());
                    lhs_type = rhs_type;
                } else if (rhs_type == SyInt) {
                    cur_function->add_instr(I::i2f());
                    rhs_type = lhs_type;
                }
            }
            assert(lhs_type == rhs_type);
            setType(exp, SyInt);

            auto instr = When(exp, {
                    TagMatch<InstructionRef>("Equal", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmpeq();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmpeq();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("NotEq", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmpne();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmpne();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Less", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmplt();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmplt();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("LessEq", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmple();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmple();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("Greater", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmpgt();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmpgt();
                                })
                        });
                    }),
                    TagMatch<InstructionRef>("GreaterEq", [&]() {
                        return When(exp, {
                                TypeIs<InstructionRef>(SyInt, [&]() {
                                    return I::icmpge();
                                }),
                                TypeIs<InstructionRef>(SyFloat, [&]() {
                                    return I::fcmpge();
                                })
                        });
                    })
            });

        }

        void translateLogic(NodeRef exp) {
            // TODO
        }

        /**
         * @deprecated No need this, 'cause the SIR is abstract stack machine.
         * @param exp
         */
        void translateTypePush(NodeRef exp) {
            panic("ERR");
        }
    };
};


#endif