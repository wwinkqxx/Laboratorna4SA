#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif
using namespace std;

const int N = 30;

struct Rec {
    int num;
    int key;
    int x, y;
};
vector<Rec> recs;
struct BNode {
    bool leaf = true;
    vector<int> keys;
    vector<int> ids;
    vector<BNode*> kids;
    BNode* next = nullptr;
};
BNode* broot = new BNode();

BNode* bInsert(BNode* n, int key, int id, int& up) {
    if (n->leaf) {
        int i = 0;
        while (i < (int)n->keys.size() && n->keys[i] < key) i++;
        n->keys.insert(n->keys.begin() + i, key);
        n->ids.insert(n->ids.begin() + i, id);
        if (n->keys.size() < 3) return nullptr;

        BNode* r = new BNode();
        r->keys.assign(n->keys.begin() + 1, n->keys.end());
        r->ids.assign(n->ids.begin() + 1, n->ids.end());
        n->keys.resize(1);
        n->ids.resize(1);
        r->next = n->next;
        n->next = r;
        up = r->keys[0];
        return r;
    }

    int i = 0;
    while (i < (int)n->keys.size() && key >= n->keys[i]) i++;
    int k;
    BNode* c = bInsert(n->kids[i], key, id, k);
    if (!c) return nullptr;

    n->keys.insert(n->keys.begin() + i, k);
    n->kids.insert(n->kids.begin() + i + 1, c);
    if (n->keys.size() < 3) return nullptr;

    BNode* r = new BNode();
    r->leaf = false;
    up = n->keys[1];
    r->keys.push_back(n->keys[2]);
    r->kids.assign(n->kids.begin() + 2, n->kids.end());
    n->keys.resize(1);
    n->kids.resize(2);
    return r;
}

void bAdd(int key, int id) {
    int up;
    BNode* r = bInsert(broot, key, id, up);
    if (r) {
        BNode* nr = new BNode();
        nr->leaf = false;
        nr->keys.push_back(up);
        nr->kids.push_back(broot);
        nr->kids.push_back(r);
        broot = nr;
    }
}

void bPrint() {
    vector<BNode*> level = { broot };
    int d = 0;
    while (!level.empty()) {
        cout << "Рівень " << d++ << ": ";
        vector<BNode*> nextLevel;
        for (BNode* n : level) {
            cout << "[";
            for (int i = 0; i < (int)n->keys.size(); i++) {
                if (i) cout << " ";
                cout << n->keys[i];
                if (n->leaf) cout << "(#" << recs[n->ids[i]].num << ")";
            }
            cout << "] ";
            for (BNode* c : n->kids) nextLevel.push_back(c);
        }
        cout << endl;
        level = nextLevel;
    }
}

int bFind(int K) {
    BNode* n = broot;
    while (!n->leaf) {
        int i = 0;
        while (i < (int)n->keys.size() && K >= n->keys[i]) i++;
        n = n->kids[i];
    }
    while (n) {
        for (int i = 0; i < (int)n->keys.size(); i++)
            if (n->keys[i] >= K) return n->ids[i];
        n = n->next;
    }
    return -1;
}

struct KNode {
    int id;
    KNode* l = nullptr;
    KNode* r = nullptr;
};
KNode* kroot = nullptr;

int coord(int id, int axis) {
    return axis == 0 ? recs[id].x : recs[id].y;
}

KNode* kInsert(KNode* n, int id, int d) {
    if (!n) {
        n = new KNode();
        n->id = id;
        return n;
    }
    int axis = d % 2;
    if (coord(id, axis) < coord(n->id, axis)) n->l = kInsert(n->l, id, d + 1);
    else n->r = kInsert(n->r, id, d + 1);
    return n;
}

void kPrint() {
    vector<KNode*> level = { kroot };
    int d = 0;
    while (!level.empty()) {
        cout << "Рівень " << d << " (ділення по " << (d % 2 == 0 ? "x" : "y") << "): ";
        vector<KNode*> nextLevel;
        for (KNode* n : level) {
            Rec& p = recs[n->id];
            cout << "(" << p.x << "," << p.y << ")#" << p.num << " ";
            if (n->l) nextLevel.push_back(n->l);
            if (n->r) nextLevel.push_back(n->r);
        }
        cout << endl;
        level = nextLevel;
        d++;
    }
}

int found = 0;
void kRange(KNode* n, int d, int x1, int y1, int x2, int y2) {
    if (!n) return;
    Rec& p = recs[n->id];
    if (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2) {
        cout << "  (" << p.x << "," << p.y << ") запис #" << p.num << ", ключ " << p.key << endl;
        found++;
    }
    int axis = d % 2;
    int lo = axis == 0 ? x1 : y1;
    int hi = axis == 0 ? x2 : y2;
    int c = coord(n->id, axis);
    if (lo < c) kRange(n->l, d + 1, x1, y1, x2, y2);
    if (hi >= c) kRange(n->r, d + 1, x1, y1, x2, y2);
}

int kFind(int x, int y) {
    KNode* n = kroot;
    int d = 0;
    while (n) {
        Rec& p = recs[n->id];
        if (p.x == x && p.y == y) return n->id;
        int q = (d % 2 == 0) ? x : y;
        n = (q < coord(n->id, d % 2)) ? n->l : n->r;
        d++;
    }
    return -1;
}

bool keyUsed(int key) {
    for (Rec& r : recs) if (r.key == key) return true;
    return false;
}
bool pointUsed(int x, int y) {
    for (Rec& r : recs) if (r.x == x && r.y == y) return true;
    return false;
}

int main() {
#ifdef _WIN32
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif
    srand((unsigned)time(0));

    int mode;
    cout << "1 - ввести з клавіатури, 2 - згенерувати: ";
    cin >> mode;

    while ((int)recs.size() < N) {
        Rec r;
        r.num = (int)recs.size() + 1;
        if (mode == 1) {
            cout << "Запис " << r.num << " (ключ x y): ";
            cin >> r.key >> r.x >> r.y;
            if (r.key < 1 || r.key > 100 || r.x < 0 || r.x > 100 || r.y < 0 || r.y > 100) {
                cout << "Неправильні значення, повторіть." << endl;
                continue;
            }
            if (keyUsed(r.key) || pointUsed(r.x, r.y)) {
                cout << "Такий ключ або точка вже є, повторіть." << endl;
                continue;
            }
        } else {
            do { r.key = rand() % 100 + 1; } while (keyUsed(r.key));
            do { r.x = rand() % 101; r.y = rand() % 101; } while (pointUsed(r.x, r.y));
        }
        recs.push_back(r);
    }

    cout << "\nЗаписи (номер: ключ, точка):" << endl;
    for (Rec& r : recs)
        cout << "#" << r.num << ": " << r.key << ", (" << r.x << "," << r.y << ")" << endl;

    for (int i = 0; i < N; i++) {
        bAdd(recs[i].key, i);
        kroot = kInsert(kroot, i, 0);
    }

    cout << "\n===== B+ дерево (m = 3) =====" << endl;
    bPrint();
    cout << "\n===== k-d дерево =====" << endl;
    kPrint();

    int choice;
    do {
        cout << "\n1 - завдання А, 2 - завдання Б, 3 - завдання В, 0 - вихід: ";
        cin >> choice;
        if (choice == 1) {
            int K;
            cout << "K = ";
            cin >> K;
            int id = bFind(K);
            if (id == -1) cout << "Немає ключа >= " << K << endl;
            else cout << "Запис #" << recs[id].num << ", ключ " << recs[id].key << endl;
        } else if (choice == 2) {
            int x, y;
            cout << "x y = ";
            cin >> x >> y;
            cout << "Вікно [" << x << "," << x + 20 << "] x [" << y << "," << y + 20 << "]:" << endl;
            found = 0;
            kRange(kroot, 0, x, y, x + 20, y + 20);
            if (found == 0) cout << "  точок немає" << endl;
        } else if (choice == 3) {
            int x, y;
            cout << "q (x y) = ";
            cin >> x >> y;
            int id = kFind(x, y);
            if (id == -1) cout << "Точки немає" << endl;
            else cout << "Запис #" << recs[id].num << ", ключ " << recs[id].key << endl;
        }
    } while (choice != 0);

    return 0;
}
