#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <cmath>
using namespace std;
#endif /* __PROGTEST__ */

template<typename T>
struct my_pair {//a pair to carry a vertex and it's parent (used to deal with successors with their parents and so on)
    T first;
    T second;
    my_pair(T fr, T sc) {
        first = fr;
        second = sc;
    }
};

#define DATA_SIZE 20

//Array of starting addresses of blocks
uint64_t blocks_addresses[30];

//Metadata
struct m_data {

    //----------------------------------------MEMBER VARIABLES-------------------------------------

    m_data * left = nullptr;
    m_data * right = nullptr;

    uint8_t depth = 1;
    uint8_t power = 0;
    bool is_free = true;
    uint8_t block_index = 0;

    //----------------------------------------PUBLIC METHODS---------------------------------------

    void init ( bool free, uint8_t input_block_index, int input_power)
    {
        left = right = nullptr;
        depth = 1;

        power = input_power;
        is_free = free;
        block_index = input_block_index;
    }

    m_data * stabilize() {
        int balance_cff = count_balance();
        m_data *res;

        if (balance_cff == -2) {
            if (left->count_balance() != 1)
                res = left_rotation();
            else {
                left = left->right_rotation();
                res = left_rotation();
            }
        } else if (balance_cff == 2) {
            if (right->count_balance() != -1)
                res = right_rotation();
            else {
                right = right->left_rotation();
                res = right_rotation();
            }
        } else {
            update_depth();
            res = this;
        }
        return res;
    }

    uint64_t add ()
    {
        return (uint64_t) this;
    }

    m_data * get_buddy ()
    {
        return (m_data *) (((add() - blocks_addresses[block_index]) ^ (uint64_t) (1 << power)) + blocks_addresses[block_index]);
    }

private:

    //----------------------------------------PRIVATE METHODS--------------------------------------

    void update_depth() {
        if (left && right)
            depth = std::max(left->depth, right->depth) + 1;
        else if (left)
            depth = left->depth + 1;
        else if (right)
            depth = right->depth + 1;
        else
            depth = 1;
    }

    int count_balance() {
        if (left && right)
            return right->depth - left->depth;
        if (left)
            return left->depth * -1;
        if (right)
            return right->depth;
        return 0;
    }

    m_data * left_rotation() {
        m_data * orig_left = left;

        left = orig_left->right;
        orig_left->right = this;

        update_depth();
        orig_left->update_depth();

        return orig_left;
    }

    m_data * right_rotation() {
        m_data * orig_right = right;

        right = orig_right->left;
        orig_right->left = this;

        update_depth();
        orig_right->update_depth();

        return orig_right;
    }
};

struct AVL_tree {

    //Resets the root of a tree to it's default value (nullptr)
    void init ()
    {
        root = nullptr;
    }

    //Returns root of a tree
    m_data * get_root ()
    {
        return root;
    }

    //Returns a node with the smallest address (the leftmost one)
    m_data * get_smallest ()
    {
        m_data * cur = root;

        while (cur->left)
            cur = cur->left;

        return cur;
    }

    //Insertion
    m_data * insert (uint64_t add, bool free, uint8_t input_blocks_index, int input_power) {
        if (!root) {
            root = (m_data *)add;
            root->init(free, input_blocks_index, input_power);
            return root;
        }
        return insert(root, add, free, input_blocks_index, input_power, nullptr, true);
    }

    //Deletion
    bool del (uint64_t to_delete) {
        if (!root)
            return false;
        return del(root, to_delete, nullptr, true);
    }

    //Returns the closest allocated memory cell to the left of a given address
    m_data * find_closest_allocated (uint64_t add)
    {
        //Initialization
        m_data * cur = root, * res = nullptr;

        while (cur)
        {
            //Go-to right case
            if(add > cur->add())
            {
                res = cur;
                cur = cur->right;
            }
                //Go-to left case
            else if (add < cur->add())
                cur = cur->left;
                //Given address is allocated node's metadata by itself -> exception
            else
                return nullptr;
        }
        return res;
    }

private:
    m_data * root = nullptr;

    //Recursive part of insertion
    m_data * insert (m_data * node, uint64_t input_add, bool free, uint8_t input_blocks_index, int input_power, m_data * prev, bool right_child) {
        m_data * res;

        if (!node) {
            res = (m_data *)input_add;
            res->init(free, input_blocks_index, input_power);
            if (right_child)
                prev->right = res;
            else
                prev->left = res;
            return res;
        }

        else if (input_add > node->add()) {
            res = insert(node->right, input_add, free, input_blocks_index, input_power, node, true);
            if (prev)  {
                if (right_child)
                    prev->right = node->stabilize();
                else
                    prev->left = node->stabilize();
            } else
                root = node->stabilize();
            return res;
        }

        else if (input_add < node->add()) {
            res = insert(node->left, input_add, free, input_blocks_index, input_power, node, false);
            if (prev) {
                if (right_child)
                    prev->right = node->stabilize();
                else
                    prev->left = node->stabilize();
            } else
                root = node->stabilize();
            return res;
        }

        else
            return nullptr;
    }

    //Recursive part of a deletion
    bool del (m_data * node, uint64_t to_delete, m_data * prev, bool right_child) {
        bool res = false;

        if (!node)
            return false;

        else if (to_delete > node->add()) {
            res = del (node->right, to_delete, node, true);
            if (prev) {
                if (right_child)
                    prev->right = node->stabilize();
                else
                    prev->left = node->stabilize();
            } else
                root = node->stabilize();
            return res;
        }

        else if (to_delete < node->add()) {
            res = del (node->left, to_delete, node, false);
            if (prev) {
                if (right_child)
                    prev->right = node->stabilize();
                else
                    prev->left = node->stabilize();
            } else
                root = node->stabilize();
            return res;
        }

        else {
            if (node->right && node->left) {
                my_pair<m_data *> succ = find_succ(node);

                swap(node, prev, succ.first, succ.second);

                res = del(succ.first->right, node->add(), succ.first, true);
                if (prev) {
                    if (right_child)
                        prev->right = succ.first->stabilize();
                    else
                        prev->left = succ.first->stabilize();
                } else
                    root = succ.first->stabilize();
                return res;
            }

            else if (node->right) {
                if (prev) {
                    if (right_child)
                        prev->right = node->right;
                    else
                        prev->left = node->right;
                } else
                    root = node->right;
                return true;
            }

            else {
                if (prev) {
                    if (right_child)
                        prev->right = node->left;
                    else
                        prev->left = node->left;
                } else
                    root = node->left;
                return true;
            }
        }
    }

    //Swaps first and second nodes, where second is a successor of first (swap is done by rearranging pointers, not by swapping values)
    void swap (m_data * first, m_data * first_prev, m_data * second, m_data * second_prev ) {
        m_data * first_right_tmp = first->right;
        m_data * first_left_tmp = first->left;

        first->right = second->right;
        first->left = second->left;
        if (first_right_tmp != second) {//second node is not a right child of the first one
            second->right = first_right_tmp;
            second->left = first_left_tmp;

            if (first_prev) {
                if (first_prev->right == first)
                    first_prev->right = second;
                else
                    first_prev->left = second;
            } else
                root = second;

            if (second_prev->right == second)
                second_prev->right = first;
            else
                second_prev->left = first;
        }
        else {//second is the right child of the first one
            second->right = first;
            second->left = first_left_tmp;

            if (first_prev) {
                if (first_prev->right == first)
                    first_prev->right = second;
                else
                    first_prev->left = second;
            } else
                root = second;
        }
    }

    //Finds a successor and a parent of a successor
    my_pair<m_data *> find_succ (m_data * node) {
        m_data * cur;
        m_data * parent = nullptr;

        parent = node;
        cur = node->right;

        while (cur->left) {
            parent = cur;
            cur = cur->left;
        }

        return {cur, parent};
    }
};

//Returns a power of 2 that can hold memory of a given size
int upper_power (int num)
{
    if (num <= DATA_SIZE) return 0;
    return (int) ceil(log2(num));
}

//Returns a power of 2 that can be fit into input memory size
int lower_power (int num)
{
    if (num <= DATA_SIZE) return 0;
    return (int) floor(log2(num));
}

//--------------------------------------------INSTANCES CREATION-----------------------------------

//Tree of allocated memory slots
AVL_tree allocated;

//Array, where indexes are powers of 2 and entries are trees of free memory slots of indexed power of 2
AVL_tree free_slots[30];

//Maximum allowed memory address
uint64_t max_add = 0;

//Counter of allocated memory slots
int alloc_cnt = 0;

//--------------------------------------------MERGE FUNCTION---------------------------------------

void merge (m_data * add)
{
    //Initialize current node's ptr
    m_data * cur = add;

    //While we can find a buddy and this buddy is free and in bounds of a Heap, we merge
    while (cur->get_buddy() < (m_data *) max_add && cur->get_buddy()->is_free && cur->get_buddy()->power == cur->power)
    {
        //Delete buddy
        free_slots[cur->get_buddy()->power].del(cur->get_buddy()->add());

        //Set cur to point at the beginning of a newly merged block
        cur = (m_data *) min(cur->add(), cur->get_buddy()->add());

        //Init cur as new, merged temporary, block
        cur->init(true, cur->block_index, cur->power + 1);
    }

    //After merging is done, we insert the resulting free memory node into free slots
    free_slots[cur->power].insert(cur->add(), true, cur->block_index, cur->power);
}

//--------------------------------------------HEAP FUNCTIONS---------------------------------------

void   HeapInit    ( void * memPool, int memSize )
{
    //Invalid input check
    if ( ! memPool || memSize <= 0) return;

    //Set current free address to the input starting address
    auto cur = (uint64_t) memPool;

    //Define max allowed memory address
    max_add = cur + memSize;

    //Set both, free and allocated memory storages to default (nullptr) + reset allocated memory counter
    alloc_cnt = 0;
    allocated.init();
    for (auto & free_slot : free_slots)
        free_slot.init();

    //Set blocks counter to zero
    uint8_t blocks_cnt = 0;

    //While memory is divisible (we can insert metadata in it)
    while (memSize > DATA_SIZE)
    {
        //Insert starting address of a block into array of block's starting addresses
        blocks_addresses[blocks_cnt] = cur;

        //Insert the greatest fitting power of 2 into it's slot
        free_slots[lower_power(memSize)].insert( cur, true, blocks_cnt++, lower_power(memSize));

        //Move current address to the next free memory address
        cur += (uint64_t) pow(2, lower_power(memSize));

        //Decrease memSize by the inserted memory size
        memSize -= pow(2, lower_power(memSize));
    }
}
void * HeapAlloc   ( int    size )
{
    //Find a free power of 2, inside which we can fit memory of requested size
    int cur_pow = upper_power(size + DATA_SIZE);
    while (cur_pow < 30 && free_slots[cur_pow].get_root() == nullptr)
        cur_pow++;

    //Validate the input
    if ( cur_pow >= 30 || size <= 0 ) return nullptr;

    //Get a free slot (the one with the smallest address, in order to improve fragmentation)
    m_data * free_slot = free_slots[cur_pow].get_smallest();

    //Divide free slot in two halves and insert the second one into free slots until the smallest fitting power of 2 is achieved
    free_slots[cur_pow].del(free_slot->add());
    while (cur_pow != upper_power(size + DATA_SIZE)) {
        cur_pow--;
        free_slots[cur_pow].insert(free_slot->add() + (uint64_t) pow(2, cur_pow), true, free_slot->block_index, cur_pow);
    }

    //Allocate memory (by inserting it into a tree of allocated memory slots) and return address right after the block's metadata
    alloc_cnt++;
    return (void *) (allocated.insert(free_slot->add(), false, free_slot->block_index, cur_pow)->add() + DATA_SIZE);
}
bool   HeapFree    ( void * blk )
{
    //Get rid of void * to allow pointer arithmetic
    auto block = (uint64_t) blk;

    //Find the closest allocated memory cell to the left of a given address
    m_data * closest = allocated.find_closest_allocated(block);

    //Validate the input
    if (! blk || ! closest || (block != closest->add() + DATA_SIZE )) return false;

    //Delete this memory cell from allocated memory list
    allocated.del(closest->add());

    //This memory cell is now free, so we merge it with it's neighbours
    merge(closest);

    alloc_cnt--;
    return true;
}
void   HeapDone    ( int  * pendingBlk )
{
    //Validate the input
    if (! pendingBlk) return;

    //Set output parameter to a number of allocated memory blocks
    *pendingBlk = alloc_cnt;

    //Reset the counter
    alloc_cnt = 0;
}


#ifndef __PROGTEST__
int main ( void )
{
    uint8_t       * p0, *p1, *p2, *p3, *p4;
    int             pendingBlk;
    static uint8_t  memPool[3 * 1048576];

    HeapInit(memPool, 320);
    assert ( (uint8_t*) HeapAlloc ( 1 )  != NULL );
    assert ( ( p0 = (uint8_t*) HeapAlloc ( 20 ) ) != NULL );
    assert ( (uint8_t*) HeapAlloc ( 1 )  != NULL );
    assert ( ( p1 = (uint8_t*) HeapAlloc ( 20 ) ) != NULL );
    assert ( (uint8_t*) HeapAlloc ( 1 )  != NULL );
    assert ( ( p2 = (uint8_t*) HeapAlloc ( 20 ) ) != NULL );
    assert ( (uint8_t*) HeapAlloc ( 1 )  != NULL );
    assert ( HeapFree ( p0 ) );
    assert ( HeapFree ( p1 ) );
    assert ( HeapFree ( p2 ) );
    assert ( ( p3 = (uint8_t*) HeapAlloc ( 21 ) ) != NULL );
    HeapDone(&pendingBlk);



    HeapInit ( memPool, 2097151 );
    assert ( ( p0 = (uint8_t*) HeapAlloc ( 512000 ) ) != NULL );
    memset ( p0, 0, 512000 );
    assert ( ( p1 = (uint8_t*) HeapAlloc ( 511000 ) ) != NULL );
    memset ( p1, 0, 511000 );
    assert ( ( p2 = (uint8_t*) HeapAlloc ( 26000 ) ) != NULL );
    memset ( p2, 0, 26000 );
    assert ( HeapFree ( p2 ) );
    assert ( HeapFree ( p0 ) );
    assert ( HeapFree ( p1 ) );
    HeapDone ( &pendingBlk );
    assert ( pendingBlk == 0 );


    HeapInit ( memPool, 2097152 );
    assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
    memset ( p0, 0, 1000000 );
    assert ( ( p1 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
    memset ( p1, 0, 250000 );
    assert ( ( p2 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
    memset ( p2, 0, 250000 );
    assert ( ( p3 = (uint8_t*) HeapAlloc ( 250000 ) ) != NULL );
    memset ( p3, 0, 250000 );
    assert ( ( p4 = (uint8_t*) HeapAlloc ( 50000 ) ) != NULL );
    memset ( p4, 0, 50000 );
    assert ( HeapFree ( p2 ) );
    assert ( HeapFree ( p4 ) );
    assert ( HeapFree ( p3 ) );
    assert ( HeapFree ( p1 ) );
    assert ( ( p1 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
    memset ( p1, 0, 500000 );
    assert ( HeapFree ( p0 ) );
    assert ( HeapFree ( p1 ) );
    HeapDone ( &pendingBlk );
    assert ( pendingBlk == 0 );


    HeapInit ( memPool, 2359296 );
    assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
    memset ( p0, 0, 1000000 );
    assert ( ( p1 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
    memset ( p1, 0, 500000 );
    assert ( ( p2 = (uint8_t*) HeapAlloc ( 500000 ) ) != NULL );
    memset ( p2, 0, 500000 );
    assert ( ( p3 = (uint8_t*) HeapAlloc ( 500000 ) ) == NULL );
    assert ( HeapFree ( p2 ) );
    assert ( ( p2 = (uint8_t*) HeapAlloc ( 300000 ) ) != NULL );
    memset ( p2, 0, 300000 );
    assert ( HeapFree ( p0 ) );
    assert ( HeapFree ( p1 ) );
    HeapDone ( &pendingBlk );
    assert ( pendingBlk == 1 );


    HeapInit ( memPool, 2359296 );
    assert ( ( p0 = (uint8_t*) HeapAlloc ( 1000000 ) ) != NULL );
    memset ( p0, 0, 1000000 );
    assert ( ! HeapFree ( p0 + 1000 ) );
    HeapDone ( &pendingBlk );
    assert ( pendingBlk == 1 );


    return 0;
}
#endif /* __PROGTEST__ */

