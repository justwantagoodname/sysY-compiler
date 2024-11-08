﻿#include "triples.h"


void Triples::minTempVar()
{
    if (temp_count == -1) {
        throw " need make() before. ";
        return;
    }
    int* var_begin = new int[temp_count + 5];
    int* var_end = new int[temp_count + 5];
    int* var_merage = new int[temp_count + 5];

    for (int i = 0; i < temp_count; ++i)var_merage[i] = i;

    std::vector<int> block_begin;

    // 置-1
    memset(var_begin, -1, sizeof(int) * temp_count);
    memset(var_end, -1, sizeof(int) * temp_count);

    auto iftin = [](int i, TripleValue* e, int* lst) {

        do {
            if (e->type == TT.temp && lst[e->value] == -1) {
                lst[e->value] = i;
                break;
            }
            e = e->added;
        } while (e);
        };

    for (int i = 0, j = triples.size() - 1; i < triples.size(); ++i, --j) {
        // 双向扫描确定每个临时变量的始末位置
//#define iftin(idx, p, lst) if(triples[idx]->p.type == TT.temp && lst[triples[idx]->p.value] == -1) \
//		lst[triples[idx]->p.value] = idx
//#undef iftin


        iftin(i, &triples[i]->e1, var_begin);
        iftin(i, &triples[i]->e2, var_begin);
        iftin(i, &triples[i]->to, var_begin);

        iftin(j, &triples[j]->e1, var_end);
        iftin(j, &triples[j]->e2, var_end);
        iftin(j, &triples[j]->to, var_end);

        //printf("%d > cmd = %d\n", i, triples[i]->cmd);

        // 找到每一个连续运行区间始点
        if (triples[i]->cmd == Cmd.tag ||
            (triples[i]->cmd >= Cmd.jmp && triples[i]->cmd <= Cmd.jle))
        {
            block_begin.push_back(i);
        }
    }
    block_begin.push_back(triples.size());

    // 遍历每一个区间，迭代合并无重叠临时变量直到不存在无合并变量
    std::vector<bool>meraged(temp_count, false);

    for (int _i = 0; _i < block_begin.size() - 1; ++_i) {
        int b = block_begin[_i];
        int e = block_begin[_i + 1];

        std::vector<int> block_temp;

        // 找到区间内临时变量
        for (int i = 0; i < temp_count; ++i) {
            if (var_begin[i] >= b && var_end[i] < e) {
                block_temp.emplace_back(i);
            }
        }

        // 采用贪心求解每个变量的最大合并占用

        // 合并标记
        //bool* meraged = new bool[block_temp.size() + 5];
        //memset(meraged, 0, block_temp.size());


        // 临时函数：查找未被合并过的离某个起点最近起始的变量
        auto found_nearlest = [var_begin, &block_temp, &meraged](int begin) {
            int min = -1;
            int min_temp = -1;
            for (auto t : block_temp) {
                if (var_begin[t] - begin >= 0 &&
                    (min == -1 || var_begin[t] - begin < min) && !meraged[t]) {
                    min = var_begin[t] - begin;
                    min_temp = t;
                }
            }
            return min_temp;
            };

        auto meraged_finish = [&meraged]() {
            for (bool i : meraged) {
                if (!i) return false;
            }
            return true;
            };

        auto merage_temp_var = [&meraged, var_merage, var_begin, var_end, this](int a, int b) {

            var_merage[b] = var_merage[a];
            var_end[a] = var_end[b];

            var_begin[b] = var_begin[a];

            meraged[b] = true;

            };

        // 合并
        while (!meraged_finish()) {

            int bottom = b;
            int nextt = found_nearlest(bottom);
            int begint = nextt;
            if (begint == -1) {
                break;
            }
            while (nextt != -1) {
                int temp = 42;
                // 终止本次合并，（在本次循环中）启动下一次
                if (
                    triples[var_begin[nextt]]->cmd == Cmd.load
                    || temp_type[begint] != temp_type[nextt]
                    ) {
                    meraged[begint] = true;
                    begint = nextt;
                }
                merage_temp_var(begint, nextt);

                bottom = var_end[nextt];
                nextt = found_nearlest(bottom);
            }
            meraged[begint] = true;
        }
    }

    auto sett = [var_merage](TripleValue* e) {
        do {
            if (e->type == TT.temp) {
                e->value = var_merage[e->value];
            }
            e = e->added;
        } while (e);
        };

    // 写回合并结果
    for (int i = 0; i < triples.size(); ++i) {
        //#define sett(p) if(triples[i]->p.type == TT.temp) triples[i]->p.value = var_merage[triples[i]->p.value]
        //#undef sett
        //printf("%d > cmd = %d\n", i, triples[i]->cmd);

        sett(&triples[i]->e1);
        sett(&triples[i]->e2);
        sett(&triples[i]->to);
    }

    delete[] var_begin;
    delete[] var_end;
    delete[] var_merage;
}

void Triples::eliUnnecVar()
{
    // 消除到立即数的无用中间变量
    std::vector<TripleValue> imd_temp(temp_count, TripleValue());

    auto sett = [f = [&imd_temp](auto&& self, TripleValue* e) -> void {

        if (e->type == TT.temp && imd_temp[e->value].type != TT.null) {
            auto added = e->added;
            *e = imd_temp[e->value];
            e->added = added;
        }
        if (e->added != nullptr) {
            self(self, e->added);
        }
        }]
        (TripleValue* e) -> void { f(f, e); };

    int i = 0;
    for (auto it = triples.begin(); it != triples.end(); ++it) {

        sett(&(*it)->e1);
        sett(&(*it)->e2);

        // 获取某个立即数并绑定到临时变量
        if ((*it)->cmd == Cmd.mov &&
            (
                (*it)->e1.type == TT.dimd
                || (*it)->e1.type == TT.fimd
                )
            && (*it)->to.type == TT.temp) {
            imd_temp[(*it)->to.value] = (*it)->e1;
            it = triples.erase(it);
            --it;
        }


        // 获取某个立即数并绑定到临时变量
        if ((
            (*it)->cmd == Cmd.add
            || (*it)->cmd == Cmd.sub
            || (*it)->cmd == Cmd.mul
            || (*it)->cmd == Cmd.div
            || (*it)->cmd == Cmd.mod
            )
            &&
            (
                (*it)->e1.type == TT.dimd
                && (*it)->e2.type == TT.dimd
                )
            && (*it)->to.type == TT.temp) {
            int a1 = (*it)->e1.value, a2 = (*it)->e2.value;
            int ans, cmd = (*it)->cmd;
            if (cmd == Cmd.add) ans = a1 + a2;
            else if (cmd == Cmd.sub) ans = a1 - a2;
            else if (cmd == Cmd.mul) ans = a1 * a2;
            else if (cmd == Cmd.div) ans = a1 / a2;
            else if (cmd == Cmd.mod) ans = a1 % a2;

            imd_temp[(*it)->to.value] = { ans, TT.dimd };
            it = triples.erase(it);
            --it;
        }
        // 当某个临时变量被重新赋值时取消绑定 (尽管可能不存在这样的情况）
        else if ((*it)->to.type == TT.temp && imd_temp[(*it)->to.value] != -1) {
            imd_temp[(*it)->to.value] = {};
        }
    }

    //delete[] imd_temp;
    //delete[] imd_type;

//	// 消除到立即数的无用中间变量
//	std::vector<int> imd_temp(temp_count, -1);
//	//int* imd_temp = new int[temp_count + 5];
//	//memset(imd_temp, -1, sizeof(int) * temp_count);
//	std::vector<bool> imd_type(temp_count, false);
//	//bool* imd_type = new bool[temp_count + 5];
//	//memset(imd_type, 0, sizeof(bool) * temp_count);
//
//	auto sett = [f = [&imd_temp, &imd_type](auto&& self, TripleValue* e) -> void {\
//
//		if (e->type == TT.temp && imd_temp[e->value] != -1) {
//
//			if (imd_type[e->value]) {
//				e->type = TT.fimd;
//			}
//			else {
//				e->type = TT.dimd;
//			}
//			e->value = imd_temp[e->value];
//
//		}
//		if (e->added != nullptr) {
//			self(self, e->added);
//		}
//		}]
//		(TripleValue* e) {\
//		f(f, e);
//	};
//
//	int i = 0;
//	for (auto it = triples.begin(); it != triples.end(); ++it) {
//		\
//		sett(&(*it)->e1);
//		sett(&(*it)->e2);
//
//		// 获取某个立即数并绑定到临时变量
//		if ((*it)->cmd == Cmd.mov && ((*it)->e1.type == TT.dimd || (*it)->e1.type == TT.fimd) && (*it)->to.type == TT.temp) {
//			imd_temp[(*it)->to.value] = (*it)->e1.value;
//			if ((*it)->e1.type == TT.fimd) {
//				imd_type[(*it)->to.value] = true;
//			}
//			it = triples.erase(it);
//			--it;
//		}
//		// 当某个临时变量被重新赋值时取消绑定 (尽管可能不存在这样的情况）
//		else if ((*it)->to.type == TT.temp && imd_temp[(*it)->to.value] != -1) {
//			imd_temp[(*it)->to.value] = -1;
//		}
//	}
//
//	//delete[] imd_temp;
//	//delete[] imd_type;
}

void Triples::resortTemp()
{
    // 重做类型传递
    int* temp_stack = new int[temp_count + 5];
    memset(temp_stack, -1, sizeof(int) * temp_count);
    int stack_top = 0;

    auto sett = [f = [temp_stack, this](auto&& self, TripleValue* e) -> void {
        if (e->type == TT.temp) {
            temp_type[temp_stack[e->value]] = temp_type[e->value];
            e->value = temp_stack[e->value];
        }
        if (e->added != nullptr) {
            self(self, e->added);
        }
        }]
        (TripleValue* e) { f(f, e); };

    for (auto it = triples.begin(); it != triples.end(); ++it) {
        if ((*it)->to.type == TT.temp && temp_stack[(*it)->to.value] == -1)
        {
            temp_stack[(*it)->to.value] = stack_top;
            ++stack_top;
        }
        sett(&(*it)->e1);
        sett(&(*it)->e2);
        sett(&(*it)->to);
    }
}


void Triples::listTempType()
{
    temp_type.resize(temp_count);
    temp_type.clear();

    for (auto e : triples) {
        switch (e->cmd)
        {
        case Cmd.load:
            temp_type[e->to.value] = getValueType(e->e1);
            break;
        case Cmd.mov:
        case Cmd.call:
            if (e->to.type == TT.temp) {
                temp_type[e->to.value] = getValueType(e->e1);
            }
            break;
        case Cmd.add:
        case Cmd.sub:
        case Cmd.mul:
        case Cmd.div:
        case Cmd.mod:
            // 判断计算后类型
            if (getValueType(e->e1) == 2 || getValueType(e->e2) == 2)
                temp_type[e->to.value] = 2;
            else
                temp_type[e->to.value] = 1;
            break;
        default:
            break;
        }
    }
}
