#ifndef DATABASE
#define DATABASE

#include <iostream>
#include <cstring>
#include <string>
#include <cstdio>
#include <cmath>
#include <fstream>
#include "utility.hpp"
#include "vector.hpp"

template<class T>
class Database {
public:
    class data {
    public:
        char key[65];
        T value;

        data() {}

        data(char *obj, const T &value_) {
            strcpy(key, obj);
            value = value_;
        }

        friend bool operator<(const data &obj1, const data &obj2) {
            if (strcmp(obj1.key, obj2.key) < 0)
                return true;
            if (strcmp(obj1.key, obj2.key) == 0 && obj1.value < obj2.value)
                return true;
            return false;
        }

        friend bool operator>(const data &obj1, const data &obj2) {
            if (strcmp(obj1.key, obj2.key) > 0)return true;
            if (strcmp(obj1.key, obj2.key) == 0 && obj1.value > obj2.value)return true;
            return false;
        }

        friend bool operator==(const data &obj1, const data &obj2) {
            if (strcmp(obj1.key, obj2.key) == 0 && obj1.value == obj2.value)
                return true;
            return false;
        }

        friend bool operator>=(const data &obj1, const data &obj2) {
            if (strcmp(obj1.key, obj2.key) > 0)return true;
            if (strcmp(obj1.key, obj2.key) == 0 && obj1.value >= obj2.value)return true;
            return false;
        }

        friend bool operator<=(const data &obj1, const data &obj2) {
            if (strcmp(obj1.key, obj2.key) < 0)return true;
            if (strcmp(obj1.key, obj2.key) == 0 && obj1.value <= obj2.value)return true;
            return false;
        }

        data operator=(const data &obj) {
            strcpy(key, obj.key);
            value = obj.value;
            return *this;
        }
    };

    static const int size_of_block = 120;

    class start {
    public:
        int rootAddr;
        int totBlocks;

        start() {
            rootAddr = 1;
            totBlocks = 1;
        }

        start(int root, int num){
            rootAddr = root;
            totBlocks = num;
        }

    };

    enum nodeType {
        index, leaf
    };

    class node {
    public:
        nodeType type;
        int pos_of_fa;
        int now_num;
        int edge[size_of_block + 1];
        data value[size_of_block];
        int front_pos, back_pos;

        node() {
            type = leaf;
            now_num = 0;
            pos_of_fa = 0;
            front_pos = 0;
            back_pos = 0;
        }
    };

    std::fstream opfile;
    start head;

    //int cnt=0;
    void getstart(start &st) {
        opfile.seekg(0);
        opfile.read(reinterpret_cast<char *>(&st), sizeof(st));
    }

    void writestart(start &st) {
        opfile.seekp(0);
        opfile.write(reinterpret_cast<char *>(&st), sizeof(st));
    }

    void getnode(node &obj, int num) {
        opfile.seekg(sizeof(start) + (num - 1) *
                                     sizeof(node));  // set the input file pointer to the given pos , waiting to extract data from the given file
        opfile.read(reinterpret_cast<char *>(&obj), sizeof(obj)); //extract data from the pointer
    }

    void writenode(node &obj, int num) {
        opfile.seekp(sizeof(start) + (num - 1) * sizeof(node));
        opfile.write(reinterpret_cast<char *>(&obj), sizeof(obj));
    }

    Database() {}

    Database(std::string name) {
        std::ifstream in;
        in.open(name);
        if (!in) {
            std::ofstream outfile(name);
            outfile.seekp(0);
            start t1;
            outfile.write(reinterpret_cast<char *>(&t1), sizeof(start));
            outfile.seekp(sizeof(start));
            node t2;
            outfile.write(reinterpret_cast<char *>(&t2), sizeof(node));
        }
        opfile.open(name);
        getstart(head);
    }

    ~Database() {
        writestart(head);
        opfile.close();
    }

    int finds(char *key, int num) {
        node temp;
        getnode(temp, num);
        if (temp.type == leaf)return num;
        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, data(key, T())) - temp.value;
        return finds(key, temp.edge[t]);
    }

    sjtu::vector <T> find(char *key) {
        //cnt++;
        int num = finds(key, head.rootAddr);
        node temp;
        getnode(temp, num);
        int t = sjtu::lower_bound(temp.value, temp.value + temp.now_num, data(key, T())) - temp.value;
        if (t == temp.now_num)
            --t;
        int x = t;
        while (true) {
            if (x == -1) {
                if (temp.back_pos == 0) {
                    x++;
                    break;
                }
                getnode(temp, temp.back_pos);
                x = temp.now_num - 1;
                if (strcmp(temp.value[x].key, key) != 0) {
                    getnode(temp, temp.front_pos);
                    x = 0;
                    break;
                }
            }
            if (strcmp(temp.value[x].key, key) != 0) {
                x++;
                break;
            }
            x--;
        }
        sjtu::vector <T> ans;
        while (true) {
            if (x == temp.now_num) {
                if (temp.front_pos == 0)break;
                getnode(temp, temp.front_pos);
                x = 0;
            }
            if (strcmp(temp.value[x].key, key) == 0)
                ans.push_back(temp.value[x].value);
            else
                break;
            x++;
        }
        return ans;
    }

    int find_pos(const data &obj, int num) {
        node temp;
        getnode(temp, num);
        if (temp.type == leaf)
            return num;
        else {
            int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, obj) - temp.value;

            return find_pos(obj, temp.edge[t]);
        }
    }

    void updatefather(int pos, int fa_pos) {
        node temp;
        getnode(temp, pos);
        temp.pos_of_fa = fa_pos;
        writenode(temp, pos);
    }

    void updateleft(int pos, int left) {
        if (pos == 0)return;
        node temp;
        getnode(temp, pos);
        temp.back_pos = left;
        writenode(temp, pos);
    }

    void updateright(int pos, int right) {
        if (pos == 0)return;
        node temp;
        getnode(temp, pos);
        temp.front_pos = right;
        writenode(temp, right);
    }

    void flashindex(int fa_pos, int son_pos, const data &obj) {
        node temp;
        getnode(temp, fa_pos);
        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, obj) - temp.value;

        for (int i = temp.now_num; i > t; i--) {
            temp.value[i] = temp.value[i - 1];
            temp.edge[i + 1] = temp.edge[i];
        }//proofed

        temp.value[t] = obj;
        temp.edge[t + 1] = son_pos;
        temp.now_num++;
        writenode(temp, fa_pos);
        if (temp.now_num == size_of_block) {
            node temp2;
            int mid = size_of_block / 2;
            temp2.now_num = mid - 1;
            for (int i = mid + 1; i < size_of_block; i++) {
                temp2.value[i - mid - 1] = temp.value[i];
                temp2.edge[i - mid - 1] = temp.edge[i];
                updatefather(temp.edge[i], head.totBlocks + 1);
            }

            temp2.edge[mid - 1] = temp.edge[size_of_block];
            updatefather(temp.edge[size_of_block], head.totBlocks + 1);
            temp.now_num = mid;
            temp2.type = index;

            if (temp.pos_of_fa == 0) {
                temp.pos_of_fa = head.totBlocks + 2;
                temp2.pos_of_fa = head.totBlocks + 2;
                writenode(temp, fa_pos);
                writenode(temp2, head.totBlocks + 1);
                head.totBlocks++;
                node temp3;

                temp3.edge[0] = fa_pos;
                temp3.edge[1] = head.totBlocks;
                temp3.value[0] = temp.value[mid];
                temp3.now_num = 1;
                temp3.type = index;
                writenode(temp3, head.totBlocks + 1);
                head.totBlocks++;
                head.rootAddr = head.totBlocks;
            } else {
                temp2.pos_of_fa = temp.pos_of_fa;
                writenode(temp, fa_pos);
                writenode(temp2, head.totBlocks + 1);
                head.totBlocks++;
                flashindex(temp.pos_of_fa, head.totBlocks, temp.value[mid]);
            }
        }
    }

    void devideleaf(int pos) {
        node temp1; // Old node waiting to be divided
        getnode(temp1, pos);

        node temp2;
        temp2.back_pos = pos;  //back_pos -> direction against the direction of leaf List
        temp2.front_pos = temp1.front_pos;

        updateleft(temp1.front_pos, head.totBlocks + 1);


        temp1.front_pos = head.totBlocks + 1;

        for (int i = 0; i < size_of_block / 2; i++)
            temp2.value[i] = temp1.value[size_of_block / 2 + i];

        temp2.now_num = size_of_block / 2;
        temp1.now_num = size_of_block / 2;
        if (temp1.pos_of_fa == 0) {
            temp1.pos_of_fa = head.totBlocks + 2;
            temp2.pos_of_fa = head.totBlocks + 2;
            writenode(temp1, pos);
            writenode(temp2, head.totBlocks + 1);
            head.totBlocks++;
            node temp3;
            temp3.now_num = 1;
            temp3.edge[0] = pos;
            temp3.edge[1] = head.totBlocks;
            temp3.value[0] = temp2.value[0];
            temp3.type = index;
            writenode(temp3, head.totBlocks + 1);
            head.totBlocks++;
            head.rootAddr = head.totBlocks;
        } else {
            temp2.pos_of_fa = temp1.pos_of_fa;
            writenode(temp1, pos);
            writenode(temp2, head.totBlocks + 1);
            head.totBlocks++;
            flashindex(temp1.pos_of_fa, head.totBlocks, temp2.value[0]);
        }
    }



    void insert(char *index, T val) {
        data obj(index, val);
        int pos = find_pos(obj, head.rootAddr);
        node temp;
        getnode(temp, pos);
        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, obj) - temp.value;

        for (int i = temp.now_num; i > t; i--)  // insert the new value to the pos t
            temp.value[i] = temp.value[i - 1];

        temp.value[t] = obj;
        temp.now_num++;
        writenode(temp, pos);
        if (temp.now_num == size_of_block)
            devideleaf(pos);
        //以下为调试信息
        //test(0);
    }

    bool judge(int pos, int fa, node &temp) {
        if (pos == 0)return false;
        getnode(temp, pos);
        if (temp.pos_of_fa != fa)return false;
        return true;
    }

    void merge(int pos, node &obj1, node &obj2, node &fa, int tx) {
        obj1.value[obj1.now_num] = fa.value[tx];
        obj1.now_num++;
        for (int i = 0; i < obj2.now_num; i++) {
            obj1.value[obj1.now_num + i] = obj2.value[i];
            obj1.edge[obj1.now_num + i] = obj2.edge[i];
            node temp;
            updatefather(obj2.edge[i], pos);
        }
        obj1.now_num += obj2.now_num;
        obj1.edge[obj1.now_num] = obj2.edge[obj2.now_num];
        updatefather(obj2.edge[obj2.now_num], pos);
    }

    void balanceindex(int pos, data delindex) {
        //如果根节点且儿子删完只剩一个则用儿子替代根
        node temp;
        getnode(temp, pos);

        if (pos == head.rootAddr && temp.now_num == 1) {
            node son;
            getnode(son, temp.edge[0]);
            son.pos_of_fa = 0;
            head.rootAddr = temp.edge[0];
            writenode(son, temp.edge[0]);
            return;
        }
        // if(temp.now_num==1)std::cout<<"it's impossible"<<std::endl;
        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, delindex) - temp.value;
        for (int i = t - 1; i < temp.now_num - 1; i++)
            temp.value[i] = temp.value[i + 1];
        for (int i = t; i < temp.now_num; i++)
            temp.edge[i] = temp.edge[i + 1];
        temp.now_num--;


        if (pos == head.rootAddr) {
            writenode(temp, pos);
            return;
        }

        if (temp.now_num < size_of_block / 2 - 1) {
            node fa;
            getnode(fa, temp.pos_of_fa);
            int tx = sjtu::upper_bound(fa.value, fa.value + fa.now_num, temp.value[0]) - fa.value;
            if (tx != fa.now_num) {
                node rightbro;
                getnode(rightbro, fa.edge[tx + 1]);
                if (rightbro.now_num >= size_of_block / 2) {
                    temp.edge[size_of_block / 2 - 1] = rightbro.edge[0];
                    updatefather(rightbro.edge[0], pos);

                    temp.value[size_of_block / 2 - 2] = fa.value[tx];
                    fa.value[tx] = rightbro.value[0];
                    for (int i = 0; i < rightbro.now_num - 1; i++) {
                        rightbro.value[i] = rightbro.value[i + 1];
                        rightbro.edge[i] = rightbro.edge[i + 1];
                    }
                    rightbro.edge[rightbro.now_num - 1] = rightbro.edge[rightbro.now_num];
                    rightbro.now_num--;
                    temp.now_num++;
                    writenode(temp, pos);
                    writenode(rightbro, fa.edge[tx + 1]);
                    writenode(fa, temp.pos_of_fa);
                } else {
                    merge(pos, temp, rightbro, fa, tx);
                    writenode(temp, pos);
                    balanceindex(temp.pos_of_fa, fa.value[tx]);//可优化，直接传tx
                }
            } else {
                node leftbro;
                if (tx == 0) {
                    std::cout << "wrong " << temp.pos_of_fa << ' ' << std::endl;
                    std::cout << fa.now_num << ' ' << fa.edge[0] << ' ' << fa.edge[1] << std::endl;
                }
                getnode(leftbro, fa.edge[tx - 1]);
                if (leftbro.now_num >= size_of_block / 2) {
                    for (int i = temp.now_num; i > 0; i--) {
                        temp.value[i] = temp.value[i - 1];
                        temp.edge[i + 1] = temp.edge[i];
                    }
                    temp.edge[1] = temp.edge[0];
                    temp.edge[0] = leftbro.edge[leftbro.now_num];
                    temp.value[0] = fa.value[fa.now_num - 1];
                    // node xx;
                    // getnode(xx,temp.edge[0]);
                    // xx.pos_of_fa=pos;
                    // writenode(xx,temp.edge[0]);
                    updatefather(temp.edge[0], pos);
                    temp.now_num++;
                    leftbro.now_num--;
                    fa.value[tx - 1] = leftbro.value[leftbro.now_num];
                    writenode(temp, pos);
                    writenode(leftbro, fa.edge[tx - 1]);
                    writenode(fa, temp.pos_of_fa);
                } else {
                    merge(fa.edge[tx - 1], leftbro, temp, fa, tx - 1);
                    writenode(leftbro, fa.edge[tx - 1]);
                    balanceindex(temp.pos_of_fa, fa.value[tx - 1]);
                }
            }
        } else writenode(temp, pos);
    }

    void balanceleaf(int pos) {
        node temp, temp_back, temp_front;
        getnode(temp, pos);
        if (!temp.back_pos && !temp.front_pos)return;

        if (!judge(temp.back_pos, temp.pos_of_fa, temp_back)) {
            getnode(temp_front, temp.front_pos);
            if (temp_front.now_num > size_of_block / 2 - 1) {
                temp.value[size_of_block / 2 - 2] = temp_front.value[0];
                for (int i = 0; i < temp_front.now_num - 1; i++)
                    temp_front.value[i] = temp_front.value[i + 1];
                temp_front.now_num--;
                temp.now_num++;
                node temp_fa;
                getnode(temp_fa, temp.pos_of_fa);
                int t = sjtu::upper_bound(temp_fa.value, temp_fa.value + temp_fa.now_num,
                                          temp.value[size_of_block / 2 - 2]) - temp_fa.value;
                temp_fa.value[t - 1] = temp_front.value[0];//修改
                writenode(temp, pos);
                writenode(temp_front, temp.front_pos);
                writenode(temp_fa, temp.pos_of_fa);
            } else {
                for (int i = size_of_block / 2 - 2; i < size_of_block - 3; i++)
                    temp.value[i] = temp_front.value[i - (size_of_block / 2 - 2)];
                temp.front_pos = temp_front.front_pos;

                updateleft(temp.front_pos, pos);
                temp.now_num = size_of_block - 3;
                writenode(temp, pos);
                balanceindex(temp.pos_of_fa, temp_front.value[0]);
            }
            return;
        }
        if (!judge(temp.front_pos, temp.pos_of_fa, temp_front)) {
            if (temp_back.now_num > size_of_block / 2 - 1) {
                for (int i = temp.now_num; i > 0; i--)
                    temp.value[i] = temp.value[i - 1];
                temp.value[0] = temp_back.value[temp_back.now_num - 1];
                temp_back.now_num--;
                temp.now_num++;

                node temp_fa;
                getnode(temp_fa, temp.pos_of_fa);
                int t = sjtu::upper_bound(temp_fa.value, temp_fa.value + temp_fa.now_num,
                                          temp.value[0]) - temp_fa.value;
                temp_fa.value[t] = temp.value[0];
                writenode(temp, pos);
                writenode(temp_back, temp.back_pos);
                writenode(temp_fa, temp.pos_of_fa);
            } else {
                for (int i = size_of_block / 2 - 1; i < size_of_block - 3; i++)
                    temp_back.value[i] = temp.value[i - size_of_block / 2 + 1];
                temp_back.front_pos = temp.front_pos;

                updateleft(temp_back.front_pos, temp.back_pos);
                temp_back.now_num = size_of_block - 3;
                writenode(temp_back, temp.back_pos);
                balanceindex(temp_back.pos_of_fa, temp.value[0]);
            }
            return;
        }
        //左右都是亲兄弟
        if (temp_back.now_num > size_of_block / 2 - 1) {
            for (int i = temp.now_num; i > 0; i--)
                temp.value[i] = temp.value[i - 1];
            temp.value[0] = temp_back.value[temp_back.now_num - 1];
            temp_back.now_num--;
            temp.now_num++;
            node temp_fa;
            getnode(temp_fa, temp.pos_of_fa);
            int t = sjtu::upper_bound(temp_fa.value, temp_fa.value + temp_fa.now_num,
                                      temp.value[0]) - temp_fa.value;
            temp_fa.value[t] = temp.value[0];
            writenode(temp, pos);
            writenode(temp_back, temp.back_pos);
            writenode(temp_fa, temp.pos_of_fa);
            return;
        }
        if (temp_front.now_num > size_of_block / 2 - 1) {
            temp.value[size_of_block / 2 - 2] = temp_front.value[0];

            for (int i = 0; i < temp_front.now_num - 1; i++)
                temp_front.value[i] = temp_front.value[i + 1];
            temp_front.now_num--;
            temp.now_num++;
            node temp_fa;
            getnode(temp_fa, temp.pos_of_fa);
            int t = sjtu::upper_bound(temp_fa.value, temp_fa.value + temp_fa.now_num,
                                      temp.value[size_of_block / 2 - 2]) - temp_fa.value;
            temp_fa.value[t - 1] = temp_front.value[0];//修改
            writenode(temp, pos);
            writenode(temp_front, temp.front_pos);
            writenode(temp_fa, temp.pos_of_fa);
            return;
        }
        for (int i = size_of_block / 2 - 2; i < size_of_block - 3; i++)
            temp.value[i] = temp_front.value[i - (size_of_block / 2 - 2)];
        temp.front_pos = temp_front.front_pos;
        updateleft(temp.front_pos, pos);
        temp.now_num = size_of_block - 3;
        writenode(temp, pos);
        balanceindex(temp.pos_of_fa, temp_front.value[0]);
    }

    void freshleft(int pos, const data &obj) {
        if (pos == 0)return;
        node temp;
        getnode(temp, pos);
        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, obj) - temp.value;
        if (t != 0) {
            temp.value[t - 1] = obj;
            writenode(temp, pos);
        } else freshleft(temp.pos_of_fa, obj);
    }

    void erase(char *key, T val) {
        data obj(key, val);
        int pos = find_pos(obj, head.rootAddr);
        node temp;
        getnode(temp, pos);

        int t = sjtu::upper_bound(temp.value, temp.value + temp.now_num, obj) - temp.value;
        //if(t==0)std::cout<<114514<<std::endl;
        if (!(temp.value[t - 1] == obj)) {
            //test(1);
            return;
        }
        t--;
        if (t == 0 && head.rootAddr != pos) {
            if (temp.now_num != 1) {
                node fa;
                getnode(fa, temp.pos_of_fa);
                int t1 = sjtu::upper_bound(fa.value, fa.value + fa.now_num, temp.value[0]) - fa.value;
                if (t1 != 0) {
                    fa.value[t1 - 1] = temp.value[1];
                    writenode(fa, temp.pos_of_fa);
                } else freshleft(fa.pos_of_fa, temp.value[1]);
            } else {//专门为块的大小为4而写
                node fa;
                getnode(fa, temp.pos_of_fa);
                node left, right;
                if (!judge(temp.back_pos, temp.pos_of_fa, left)) {
                    getnode(right, temp.front_pos);
                    int t1 = sjtu::upper_bound(fa.value, fa.value + fa.now_num, temp.value[0]) - fa.value;
                    if (t1 != 0) {
                        fa.value[t1 - 1] = right.value[0];
                        writenode(fa, temp.pos_of_fa);
                    } else freshleft(fa.pos_of_fa, right.value[0]);
                } else if (!judge(temp.front_pos, temp.pos_of_fa, right)) {
                    int t1 = sjtu::upper_bound(fa.value, fa.value + fa.now_num, temp.value[0]) - fa.value;
                    if (t1 != 0) {
                        fa.value[t1 - 1] = left.value[left.now_num - 1];
                        writenode(fa, temp.pos_of_fa);
                    } else freshleft(fa.pos_of_fa, left.value[left.now_num - 1]);
                } else {
                    getnode(right, temp.front_pos);
                    int t1 = sjtu::upper_bound(fa.value, fa.value + fa.now_num, temp.value[0]) - fa.value;
                    if (t1 != 0) {
                        fa.value[t1 - 1] = right.value[0];
                        writenode(fa, temp.pos_of_fa);
                    } else freshleft(fa.pos_of_fa, right.value[0]);
                }
            }
        }

        for (int i = t; i < temp.now_num - 1; i++)
            temp.value[i] = temp.value[i + 1];

        temp.now_num--;
        writenode(temp, pos);
        if (temp.now_num < size_of_block / 2 - 1)
            balanceleaf(pos);
        //test(1);
    }
};

#endif