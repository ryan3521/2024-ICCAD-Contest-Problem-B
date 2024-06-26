#include <iostream>
#include <list>
#include <vector>
#include <fstream>
#include <string>

using namespace std;

class node;
class box{
    public:
        string name;
        double coox;
        double cooy;
        double sizex;
        double sizey;
        double min_x;
        double max_x;
        double min_y;
        double max_y;
        node* to_node;
};

class edge{
    public:
        int type; // 0 or 1
        box* to_box;
        double coor;
};

static bool cmp(edge* a, edge *b){
    if(a->coor == b->coor){
        return a->type > b->type;
    }
    else{
        return a->coor < b->coor;
    }
}


class node{
    public:
        box* b;
        node* next_node;
        node* front_node;
        // --------------
        node(box*);
};

node::node(box* b){
    this->b = b;
    next_node  = NULL;
    front_node = NULL;
}

class dLinkList{
    public:
        int size;
        node* first_node;
        node* last_node;
        // ------------
        dLinkList();
        void addNode(node*);
        void deleteNode(node*);
};

dLinkList::dLinkList(){
    size = 0;
    first_node = NULL;
    last_node  = NULL;
}

void dLinkList::addNode(node* n){
    if(size == 0){
        first_node = n;
        last_node  = n;
        n->front_node = NULL;
        n->next_node  = NULL;
        size++;
        return;
    }
    else{
        n->front_node = last_node;
        n->next_node  = NULL;
        last_node->next_node = n;
        last_node = n;
        size++;
        return;
    }
}

void dLinkList::deleteNode(node* n){
    if(n == first_node && n == last_node){
        first_node = NULL;
        last_node  = NULL;
    }
    else if(n == first_node){
        n->next_node->front_node = NULL;
        first_node = n->next_node;
    }
    else if(n == last_node){
        n->front_node->next_node = NULL;
        last_node = n->front_node;
    }
    else{
        n->front_node->next_node = n->next_node;
        n->next_node->front_node = n->front_node;
    }
    size--;
    delete n;
}

int main(int argc, char *argv[]){
    fstream fin;
    string filename = argv[1];
    list<box*> box_list;
    list<edge*> horizon;
    box* b;
    box* newb;
    edge* e;
    dLinkList my_list;
    node* itr;
    node* new_node;
    string title;
    int cell_num;
    cout << "start checking ... " << endl;
    fin.open(filename, ios::in);
    if(fin.is_open()) cout << filename << " is OPEN ..." << endl;
    else {
        cout << filename << " CANNOT OPEN -" << endl;
        return 0;
    }

    fin >> title >> cell_num ;
    for(int i=0; i<cell_num; i++){
        b = new box;
        fin >> b->name >> b->coox >> b->cooy >> b->sizex >> b->sizey;

        b->min_x = b->coox;
        b->max_x = b->coox + b->sizex;
        b->min_y = b->cooy;
        b->max_y = b->cooy + b->sizey;
        box_list.push_back(b);
        // -------------------
        e = new edge;
        e->type = 0;
        e->to_box = b;
        e->coor = b->min_x;
        horizon.push_back(e);
        // -------------------
        e = new edge;
        e->type = 1;
        e->to_box = b;
        e->coor = b->max_x;
        horizon.push_back(e);
    }
    fin.close();
    cout << "Read file done ..." << endl;

    cout << "Cell num " << box_list.size() << endl;
    horizon.sort(cmp);
    // cout << horizon.size() << endl;

    for(auto i: horizon){
        if(i->type == 0){
            newb = i->to_box;
            itr = my_list.first_node;
            while(itr!=NULL){
                b = itr->b;
                // check if newb and b overlap in verticle direction
                if((b->min_y>=newb->max_y || newb->min_y>=b->max_y) == false){
                    cout << endl << "ERROR: " << b->name << " overlap with " << newb->name << endl;
                    cout << b->name << " min x" << b->min_x << endl;
                    cout << b->name << " MAX x" << b->max_x << endl;
                    cout << "-------------------------------" << endl;
                    cout << newb->name << " min x" << newb->min_x << endl;
                    cout << newb->name << " MAX x" << newb->max_x << endl;
                    // return 0;
                }
                itr = itr->next_node;
            }
            new_node = new node(newb);
            newb->to_node = new_node;
            // cout << "add node" << endl;
            my_list.addNode(new_node);
        }
        else{
            // cout << "delete node" << endl;
            my_list.deleteNode(i->to_box->to_node);
        }
    }

    cout << "Check Done - ";
    cout << "Legal solution, no overlapped!" << endl;


    return 0;
}