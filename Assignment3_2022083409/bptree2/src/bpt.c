#include "bpt.h"

H_P * hp;

page * rt = NULL; //root is declared as global

int fd = -1; //fd is declared as global

char find_result_value[120];

//i 번째 bit는 i 번째 records/b_f를 나타냄
#define IS_DELETED(p, i) ((p->reserved[(i)/8] & (1 << ((i)%8))) != 0)
#define SET_DELETE(p, i) (p->reserved[(i)/8] |= (1 << ((i)%8)))
#define UNSET_DELETE(p, i) (p->reserved[(i)/8] &= ~(1 << ((i)%8)))

off_t find_leaf(int64_t key);
int insert_into_new_root(off_t lco, int64_t new_key, off_t rco);
int get_child_index(page * parent, off_t po); //po = parent offset
int insert_into_parent(off_t lco, int64_t new_key, off_t rco);
int adjust_root(off_t ro);
int delete_entry(off_t po, int64_t key); //po = page offset
int coalesce_pages(off_t po, off_t no, off_t ppo, int ci); //po = page offset, ppo = parent offset
int redistribute_pages(off_t po, off_t no, off_t ppo, int ci, int kkey_idx, int64_t kkey);
int write_parent(page * parent, off_t child_off, int64_t key);
void update_children_parent(page * parent, off_t parent_off);

void free_page(page * p) {
    if (p != NULL && p!= rt)
        free(p);
}

H_P * load_header(off_t off) {
    H_P * newhp = (H_P*)calloc(1, sizeof(H_P));
    if (sizeof(H_P) > pread(fd, newhp, sizeof(H_P), 0)) {

        return NULL;
    }
    return newhp;
}


page * load_page(off_t off) {
    if (!off) return NULL;
    if (rt != NULL && off == hp->rpo){
        return rt;
    }
    page* load = (page*)calloc(1, sizeof(page));
    //if (off % sizeof(page) != 0) printf("load fail : page offset error\n");
    if (sizeof(page) > pread(fd, load, sizeof(page), off)) {

        return NULL;
    }
    return load;
}

int open_table(char * pathname) {
    fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC  , 0775);
    hp = (H_P *)calloc(1, sizeof(H_P));
    if (fd > 0) {
        //printf("New File created\n");
        hp->fpo = 0;
        hp->num_of_pages = 1;
        hp->rpo = 0;
        pwrite(fd, hp, sizeof(H_P), 0);
        free(hp);
        hp = load_header(0);
        return 0;
    }
    fd = open(pathname, O_RDWR|O_SYNC);
    if (fd > 0) {
        //printf("Read Existed File\n");
        if (sizeof(H_P) > pread(fd, hp, sizeof(H_P), 0)) {
            return -1;
        }
        off_t r_o = hp->rpo;
        rt = load_page(r_o);
        return 0;
    }
    else return -1;
}

void reset(off_t off) {
    page * reset;
    reset = (page*)calloc(1, sizeof(page));
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), off);
    free(reset);
    return;
}

void freetouse(off_t fpo) {
    page * reset;
    reset = load_page(fpo);
    reset->parent_page_offset = 0;
    reset->is_leaf = 0;
    reset->num_of_keys = 0;
    reset->next_offset = 0;
    pwrite(fd, reset, sizeof(page), fpo);
    free(reset);
    return;
}

void usetofree(off_t wbf) {
    page * utf = load_page(wbf);
    utf->parent_page_offset = hp->fpo;
    utf->is_leaf = 0;
    utf->num_of_keys = 0;
    utf->next_offset = 0;
    pwrite(fd, utf, sizeof(page), wbf);
    free(utf);
    hp->fpo = wbf;
    pwrite(fd, hp, sizeof(hp), 0);
    free(hp);
    hp = load_header(0);
    return;
}

off_t new_page() {
    off_t newp;
    page * np;
    off_t prev;
    if (hp->fpo != 0) {
        newp = hp->fpo;
        np = load_page(newp);
        hp->fpo = np->parent_page_offset;
        pwrite(fd, hp, sizeof(hp), 0);
        free(hp);
        hp = load_header(0);
        free_page(np);
        freetouse(newp);
        return newp;
    }
    //change previous offset to 0 is needed
    newp = lseek(fd, 0, SEEK_END);
    //if (newp % sizeof(page) != 0) printf("new page made error : file size error\n");
    reset(newp);
    hp->num_of_pages++;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    return newp;
}



int cut(int length) {
    if (length % 2 == 0)
        return length / 2;
    else
        return length / 2 + 1;
}



void start_new_file(record rec) {

    page * root;
    off_t ro;
    ro = new_page();
    rt = load_page(ro);
    hp->rpo = ro;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    rt->num_of_keys = 1;
    rt->is_leaf = 1;
    rt->records[0] = rec;
    rt->next_offset = 0;
    pwrite(fd, rt, sizeof(page), hp->rpo);
    // free(rt);
    // rt = load_page(hp->rpo);
    //printf("new file is made\n");
}

off_t find_leaf(int64_t key) {
    off_t co = hp->rpo;
    if (co == 0) return 0;
    page * cur = load_page(co);
    if (cur == NULL) return 0;
    while (cur->is_leaf == 0){
        int i = 0;
        while (i < cur->num_of_keys && key >= cur->b_f[i].key)
            i++;
        if (i == 0) co = cur->next_offset;
        else co = cur->b_f[i - 1].p_offset;
        free_page(cur);
        cur = load_page(co);
        if (cur == NULL) return 0;
    }
    free_page(cur);
    return co;
}

char * db_find(int64_t key) {
    off_t lo = find_leaf(key);
    if (lo == 0) return NULL;
    page * leaf = load_page(lo);
    if (leaf == NULL) return NULL;
    for (int i = 0; i < leaf->num_of_keys; i++){
        if (leaf->records[i].key == key){
            // bitmap에서 bit가 1이면 삭제된 것으로 판단
            if (IS_DELETED(leaf, i)) {
                free_page(leaf);
                return NULL;
            }
            char * result = (char *)malloc(sizeof(char) * 120);
            strcpy(result, leaf->records[i].value);
            free_page(leaf);
            return result;
        }
    }
    free_page(leaf);
    return NULL;
}

//lco = left child offset, rco = right child offset
int insert_into_new_root(off_t lco, int64_t new_key, off_t rco) {
    off_t new_ro = new_page();
    page * new_rt = load_page(new_ro);
    page * left_child = load_page(lco);
    page * right_child = load_page(rco);
    new_rt->is_leaf = 0;
    new_rt->num_of_keys = 1;
    new_rt->next_offset = lco;
    new_rt->b_f[0].key = new_key;
    new_rt->b_f[0].p_offset = rco;

    left_child->parent_page_offset = new_ro;
    right_child->parent_page_offset = new_ro;

    pwrite(fd, new_rt, sizeof(page), new_ro);
    pwrite(fd, left_child, sizeof(page), lco);
    pwrite(fd, right_child, sizeof(page), rco);

    hp->rpo = new_ro;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);

    rt = new_rt;
    free_page(left_child);
    free_page(right_child);
    return 0;
}

int get_child_index(page * parent, off_t po) {
    if (po == parent->next_offset) return -1;
    for (int i = 0; i < parent->num_of_keys; i++) {
        if (parent->b_f[i].p_offset == po) return i;
    }
    return -2;
}

//lco = left child offset, rco = right child offset
int insert_into_parent(off_t lco, int64_t new_key, off_t rco) {
    page * left_child = load_page(lco);
    off_t po = left_child->parent_page_offset;
    free_page(left_child);

    if (po == 0) // root에서 split 했을 경우
        return insert_into_new_root(lco, new_key, rco);
    
    page * parent = load_page(po);
    int left_index = get_child_index(parent, lco);
    if (left_index <= -2) return -1;
    int insert_index = left_index + 1;

    if (parent->num_of_keys < INTERNAL_MAX) { //parent에 여유 공간 있을 때
        memmove(&parent->b_f[insert_index + 1], &parent->b_f[insert_index], (parent->num_of_keys - insert_index) * sizeof(I_R));
        parent->b_f[insert_index].key = new_key;
        parent->b_f[insert_index].p_offset = rco;
        parent->num_of_keys++;
        pwrite(fd, parent, sizeof(page), po);
        free_page(parent);
        return 0;
    }
    I_R tmp_bf[INTERNAL_MAX + 1];
    off_t tmp_next_offset = parent->next_offset;
    memcpy(tmp_bf, parent->b_f, parent->num_of_keys * sizeof(I_R));
    memmove(&tmp_bf[insert_index + 1], &tmp_bf[insert_index], (parent->num_of_keys - insert_index) * sizeof(I_R));
    tmp_bf[insert_index].key = new_key;
    tmp_bf[insert_index].p_offset = rco;

    int split = cut(INTERNAL_MAX + 1);
    int64_t kkey = tmp_bf[split - 1].key;       // 위로 올라갈 Key
    off_t new_next_offset = tmp_bf[split - 1].p_offset; // 오른쪽 자식의 p0
    off_t new_io = new_page(); //new internal page offset
    page * new_internal = load_page(new_io);
    new_internal->parent_page_offset = parent->parent_page_offset;
    new_internal->is_leaf = 0;
    
    parent->num_of_keys = split - 1;
    parent->next_offset = tmp_next_offset; //원래 p0 유지
    memcpy(parent->b_f, tmp_bf, parent->num_of_keys * sizeof(I_R));
    memset(&parent->b_f[parent->num_of_keys], 0, (INTERNAL_MAX - parent->num_of_keys) * sizeof(I_R)); // 남은 공간 정리

    new_internal->num_of_keys = INTERNAL_MAX + 1 - split;
    new_internal->next_offset = new_next_offset;
    memcpy(new_internal->b_f, &tmp_bf[split], new_internal->num_of_keys * sizeof(I_R));

    page * child = load_page(new_internal->next_offset); // 새로운 internal page가 생겼으므로
    if (child) {                                         // 해당 page가 가리키는 leaf page를 전부 update
        child->parent_page_offset = new_io;
        pwrite(fd, child, sizeof(page), new_internal->next_offset);
        free_page(child);
    }
    for(int i = 0; i < new_internal->num_of_keys; i++) {
        child = load_page(new_internal->b_f[i].p_offset);
        if (child) {
            child->parent_page_offset = new_io;
            pwrite(fd, child, sizeof(page), new_internal->b_f[i].p_offset);
            free_page(child);
        }
    }
    pwrite(fd, parent, sizeof(page), po);
    pwrite(fd, new_internal, sizeof(page), new_io);

    off_t ppo = parent->parent_page_offset; 
    free_page(parent);
    free_page(new_internal);
    return insert_into_parent(po, kkey, new_io);
}

int db_insert(int64_t key, char * value) {
    record new_rec;
    off_t lo;
    new_rec.key = key;
    strncpy(new_rec.value, value, 120);
    if(hp->rpo == 0) {
        start_new_file(new_rec);
        return 0;
    }

    else lo = find_leaf(key);
    page * leaf = load_page(lo);
    if (leaf == NULL) return -1;
    for (int i = 0; i < leaf->num_of_keys; i++) {
        if(leaf->records[i].key == key) {
            // bit 해제로 삭제된 키를 다시 삽입
            if (IS_DELETED(leaf, i)) {
                strncpy(leaf->records[i].value, value, 120);
                UNSET_DELETE(leaf, i);
                pwrite(fd, leaf, sizeof(page), lo);
                free_page(leaf);
                return 0;
            }
            free_page(leaf);
            return -1; // 중복 방지
        }
    }
    if (leaf->num_of_keys < LEAF_MAX){ // 여유 공간 있을 때
        int i = 0;
        while ( i < leaf->num_of_keys && leaf->records[i].key < key)
            i++;
        memmove(&leaf->records[i + 1], &leaf->records[i], (leaf->num_of_keys - i) * sizeof(record));
        uint64_t bitmap = *(uint64_t *)leaf->reserved; //reserved 배열의 앞 8byte를 64 bit 정수로 변환
        uint64_t mask = (1ULL << i) - 1; // 0번째 bit부터 i-1번째 bit까지 1
        uint64_t extended_bitmap = (bitmap & mask) | ((bitmap & ~mask) << 1); // 상위 bit를 왼쪽으로 밀고 하위 bit를 합침, records가 64개 미만이라 가능
        leaf->records[i] = new_rec;
        leaf->num_of_keys++;
        pwrite(fd, leaf, sizeof(page), lo);
        free_page(leaf);
        return 0;
    }
    record tmp_records[LEAF_MAX + 1]; //split
    int i = 0;
    while ( i < LEAF_MAX && leaf->records[i].key < key)
        i++;
    memcpy(tmp_records, leaf->records, i * sizeof(record));
    tmp_records[i] = new_rec;
    memcpy(&tmp_records[i + 1], &leaf->records[i], (LEAF_MAX - i) * sizeof(record));

    //bitmap 처리
    uint64_t bitmap = *(uint64_t *)leaf->reserved; //reserved 배열의 앞 8byte를 64 bit 정수로 변환
    uint64_t mask = (1ULL << i) - 1; // 0번째 bit부터 i-1번째 bit까지 1
    uint64_t extended_bitmap = (bitmap & mask) | ((bitmap & ~mask) << 1); // 상위 bit를 왼쪽으로 밀고 하위 bit를 합침, records가 64개 미만이라 가능

    off_t new_lo = new_page();
    page * new_leaf = load_page(new_lo);
    new_leaf->parent_page_offset = leaf->parent_page_offset;
    new_leaf->is_leaf = 1;
    new_leaf->next_offset = leaf->next_offset;
    leaf->next_offset = new_lo;

    int split = cut(LEAF_MAX + 1);
    leaf->num_of_keys = split;
    memcpy(leaf->records, tmp_records, split * sizeof(record));
    memset(&leaf->records[split], 0, (LEAF_MAX - split) * sizeof(record));
    new_leaf->num_of_keys = LEAF_MAX + 1 - split;
    memcpy(new_leaf->records, &tmp_records[split], new_leaf->num_of_keys * sizeof(record));

    //bitmap 처리
    *(uint64_t *)leaf->reserved = extended_bitmap & ((1ULL << split) - 1); //하위 split 개 bit만
    *(uint64_t *)new_leaf->reserved = extended_bitmap >> split; // 상위 bit들 가져와서 0번부터 시작하도록 내림

    pwrite(fd, leaf, sizeof(page), lo);
    pwrite(fd, new_leaf, sizeof(page), new_lo);

    int64_t new_key = new_leaf->records[0].key;
    
    free_page(leaf);
    free_page(new_leaf);

    return insert_into_parent(lo, new_key, new_lo);
}

int adjust_root(off_t ro) { 
    page * root = load_page(ro); //rt로 됨
    if (!root->is_leaf && root->num_of_keys == 0) {
        off_t new_ro = root->next_offset;
        hp->rpo = new_ro;
        if (rt != NULL) {
            free(rt);
            rt = NULL;
        }
        if (hp->rpo != 0) {
            rt = load_page(hp->rpo);
            rt->parent_page_offset = 0;
            pwrite(fd, rt, sizeof(page), hp->rpo);
        } else {
            rt = NULL;
        }
    } else if (root->is_leaf && root->num_of_keys == 0) {
        hp->rpo = 0;
        if (rt) {
            free(rt);
            rt = NULL;
        }
    } else {
        free_page(root);
        return 0;
    }
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    usetofree(ro);
    return 0;
}

//po = page offset, no = neighbor offset, 
//ppo = parent offset, ci = child index
int coalesce_pages(off_t po, off_t no, off_t ppo, int ci) {
    off_t lo, ro; //left, right offset
    int kkey_idx;
    if(ci == -1) { //가장 왼쪽의 경우 neighbor는 오른쪽
        lo = po; ro = no; kkey_idx = 0;
    } else { //그 외에는 neighbor가 왼쪽
        lo = no; ro = po; kkey_idx = ci;
    }
    page * parent = load_page(ppo);
    int64_t kkey = parent->b_f[kkey_idx].key;
    free_page(parent);
    page * left = load_page(lo);
    page * right = load_page(ro);
    if (left->is_leaf) {
        memcpy(&left->records[left->num_of_keys], right->records, right->num_of_keys * sizeof(record));
        left->num_of_keys += right->num_of_keys;
        left->next_offset = right->next_offset;
    } else {
        left->b_f[left->num_of_keys].key = kkey;
        left->b_f[left->num_of_keys].p_offset = right->next_offset;
        left->num_of_keys++;
        memcpy(&left->b_f[left->num_of_keys], right->b_f, right->num_of_keys * sizeof(I_R));
        left->num_of_keys += right->num_of_keys;
        page * child = load_page(right->next_offset);
        if (child) {
            child->parent_page_offset = lo;
            pwrite(fd, child, sizeof(page), right->next_offset);
            free_page(child);
        }
        for (int i = 0; i < right->num_of_keys; i++) {
            child = load_page(right->b_f[i].p_offset);
            if (child) {
                child->parent_page_offset = lo;
                pwrite(fd, child, sizeof(page), right->b_f[i].p_offset);
                free_page(child);
            }
        }
    }
    pwrite(fd, left, sizeof(page), lo);
    free_page(left); free_page(right);
    usetofree(ro);
    return delete_entry(ppo, kkey);
}

int redistribute_pages(off_t po, off_t no, off_t ppo, int ci, int kkey_idx, int64_t kkey) {
    page * p = load_page(po);
    page * neighbor = load_page(no);
    page * parent = load_page(ppo);
    if (ci == -1) { //가장 왼쪽의 경우 neighbor는 오른쪽
        if (p->is_leaf) {
            p->records[p->num_of_keys] = neighbor->records[0];
            memmove(&neighbor->records[0], &neighbor->records[1], (neighbor->num_of_keys - 1) * sizeof(record));
            neighbor->num_of_keys--;
            p->num_of_keys++;
            parent->b_f[kkey_idx].key = neighbor->records[0].key;
        } else {
            p->b_f[p->num_of_keys].key = kkey;
            p->b_f[p->num_of_keys].p_offset = neighbor->next_offset;
            p->num_of_keys++;
            parent->b_f[kkey_idx].key = neighbor->b_f[0].key;
            neighbor->next_offset = neighbor->b_f[0].p_offset;
            memmove(&neighbor->b_f[0], &neighbor->b_f[1], (neighbor->num_of_keys - 1) * sizeof(I_R));
            neighbor->num_of_keys--;
            page * child = load_page(p->b_f[p->num_of_keys - 1].p_offset);
            if (child) {
                child->parent_page_offset = po;
                pwrite(fd, child, sizeof(page), p->b_f[p->num_of_keys - 1].p_offset);
                free_page(child);
            }
        }
    } else { //그 외에는 neighbor가 왼쪽
        if (p->is_leaf) {
            memmove(&p->records[1], &p->records[0], p->num_of_keys * sizeof(record));
            p->records[0] = neighbor->records[neighbor->num_of_keys - 1];
            p->num_of_keys++;
            neighbor->num_of_keys--;
            parent->b_f[kkey_idx].key = p->records[0].key;
        } else {
            memmove(&p->b_f[1], &p->b_f[0], p->num_of_keys * sizeof(I_R));
            p->b_f[0].key = kkey;
            p->b_f[0].p_offset = p->next_offset;
            p->next_offset = neighbor->b_f[neighbor->num_of_keys - 1].p_offset;
            parent->b_f[kkey_idx].key = neighbor->b_f[neighbor->num_of_keys - 1].key;
            p->num_of_keys++;
            neighbor->num_of_keys--;
            page * child = load_page(p->next_offset);
            if (child) {
                child->parent_page_offset = po;
                pwrite(fd, child, sizeof(page), p->next_offset);
                free_page(child);
            }
        }
    }
    pwrite(fd, p, sizeof(page), po);
    pwrite(fd, neighbor, sizeof(page), no);
    pwrite(fd, parent, sizeof(page), ppo);
    free_page(p); free_page(neighbor); free_page(parent);
    return 0;
}

//po = page offset
int delete_entry(off_t po, int64_t key) {
    page * p = load_page(po);
    if (p->is_leaf) {
        int i = 0;
        while(p->records[i].key != key)
            i++;
        memmove(&p->records[i], &p->records[i + 1], (p->num_of_keys - i - 1) * sizeof(record));
        p->num_of_keys--;
    } else {
        int i = 0;
        while(p->b_f[i].key != key)
            i++;
        memmove(&p->b_f[i], &p->b_f[i + 1], (p->num_of_keys - i - 1) * sizeof(I_R));
        p->num_of_keys--;
    }
    pwrite(fd, p, sizeof(page), po);

    if (p->parent_page_offset == 0){
        free_page(p);
        return adjust_root(po);
    }

    int min_keys = p->is_leaf ? cut(LEAF_MAX) : cut(INTERNAL_MAX + 1) - 1;
    //internal page의 경우 pointer 기준으로 최소치를 정하기에 최소 pointer 개수에서 1을 빼야
    //최소 key 개수가 됨.
    if (p->num_of_keys >= min_keys) {
        free_page(p);
        return 0;
    }

    //page에 저장된 key 개수가 적을 경우
    //ppo = parent page offset
    off_t ppo = p->parent_page_offset;
    page * parent = load_page(ppo);
    int ci = get_child_index(parent, po); //child index
    off_t no; //neighbor offset
    int kkey_idx;
    if (ci == -1) { //가장 왼쪽의 경우 neighbor는 오른쪽
        kkey_idx = 0;
        no = parent->b_f[0].p_offset;
    } else {
        kkey_idx = ci; //그 외에는 neighbor가 왼쪽
        if (ci == 0) no = parent->next_offset;
        else no = parent->b_f[ci - 1].p_offset;
    }
    int64_t kkey = parent->b_f[kkey_idx].key;
    page * neighbor = load_page(no);
    int max_key = p->is_leaf ? LEAF_MAX : INTERNAL_MAX;
    free_page(p); free_page(parent); free_page(neighbor);
    if (neighbor->num_of_keys + p->num_of_keys <= max_key) { 
        //neighbor와 merge가 가능한 경우
        coalesce_pages(po, no, ppo, ci);
    } else {
        //neighbor와 merge가 불가능한 경우
        redistribute_pages(po, no, ppo, ci, kkey_idx, kkey);
    }
}

int db_delete(int64_t key) {
    off_t lo = find_leaf(key); //db_find 변형해서 사용
    if (lo == 0) return -1;
    int i = 0;
    page * leaf = load_page(lo);
    if (leaf == NULL) return -1;
    for (i = 0; i < leaf->num_of_keys; i++){
        if (leaf->records[i].key == key) {
            if (IS_DELETED(leaf, i)) { // 이미 삭제됨
                free_page(leaf);
                return -1;
            }
            SET_DELETE(leaf, i); //bitmap에 bit 설정
            pwrite(fd, leaf, sizeof(page), lo);
            free_page(leaf);
            return 0;
        }
    }

    free_page(leaf); //여기까지 db_find
    return -1;
}

// 부모 페이지에 자식 정보를 추가하는 함수 (reorganize)
int write_parent(page * parent, off_t child_off, int64_t key) {
    if (parent->next_offset == 0 && parent->num_of_keys == 0) {
        parent->next_offset = child_off;
        *(int64_t *)parent->reserved = key; // 이 page의 대표 key를 임시 저장
                                            // internal node의 reserved 영역은 bitmap으로 쓰이지 않음
        return 0;
    }
    int i = parent->num_of_keys;
    parent->b_f[i].key = key;
    parent->b_f[i].p_offset = child_off;
    parent->num_of_keys++;
    return 0;
}

// 자식 page들의 parent page offset을 일괄 update
void update_children_parent(page * parent, off_t parent_off) {
    page * child;
    
    // p0 자식 update
    if (parent->next_offset != 0) {
        child = load_page(parent->next_offset);
        if (child) {
            child->parent_page_offset = parent_off;
            pwrite(fd, child, sizeof(page), parent->next_offset);
            free_page(child);
        }
    }

    // 나머지 자식들 update
    for (int i = 0; i < parent->num_of_keys; i++) {
        child = load_page(parent->b_f[i].p_offset);
        if (child) {
            child->parent_page_offset = parent_off;
            pwrite(fd, child, sizeof(page), parent->b_f[i].p_offset);
            free_page(child);
        }
    }
}

void db_reorganize() {
    if (hp->num_of_pages <= 1) return;

    // 삭제되지 않은 record 추출
    int temp_record = open("temp_record.dat", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (temp_record < 0) return;

    off_t co = hp->rpo;
    if (co != 0) {
        page * cur = load_page(co);
        while(!cur->is_leaf) {
            off_t next = cur->next_offset;
            free_page(cur);
            co = next;
            cur = load_page(co);
        }
        while(co != 0 && cur != NULL) {
            for(int i=0; i<cur->num_of_keys; i++) {
                if (!IS_DELETED(cur, i)) {
                    write(temp_record, &cur->records[i], sizeof(record));
                }
            }
            off_t next = cur->next_offset;
            free_page(cur);
            co = next;
            if (co != 0) cur = load_page(co);
        }
    }
    close(temp_record);

    // DB 초기화
    if (rt != NULL) { free(rt); rt = NULL; }
    if (hp != NULL) { free(hp); hp = NULL; }
    
    ftruncate(fd, 0); 
    
    hp = (H_P *)calloc(1, sizeof(H_P));
    hp->fpo = 0;
    hp->num_of_pages = 1;
    hp->rpo = 0;
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);

    // bottom up 방식 구축
    temp_record = open("temp_record.dat", O_RDONLY);
    if (temp_record < 0) return;

    // level 0 ~ 9 까지만 고려 (충분히 큼)
    page * p[10]; 
    for(int i=0; i<10; i++) p[i] = (page *)calloc(1, sizeof(page));
    p[0]->is_leaf = 1;

    // npo = next page offset
    off_t npo[10] = {0, }; 
    for(int i=0; i<10; i++) npo[i] = new_page(); 
    
    int max_level = 0;
    record rec;
    
    while(read(temp_record, &rec, sizeof(record)) == sizeof(record)) {
        // leaf(level 0)에 record 추가
        page * leaf = p[0];
        leaf->records[leaf->num_of_keys] = rec;
        leaf->num_of_keys++;
        
        // leaf가 꽉 차면 flush
        if (leaf->num_of_keys == LEAF_MAX) {
            off_t co = npo[0];
            off_t so = new_page(); // 다음 sibling를 위해 미리 할당, so = sibling offset
            
            leaf->next_offset = so; // sibling 연결
            pwrite(fd, leaf, sizeof(page), co);
            
            // 부모(level 1)로 전파 준비
            int64_t key_to_promote = leaf->records[0].key;
            off_t child_off = co;
            
            // leaf buffer 초기화
            memset(leaf, 0, sizeof(page));
            leaf->is_leaf = 1;
            npo[0] = so; // 다음엔 여기다 씀
            
            // 상위 level 처리 (propagation)
            for (int h = 1; h < 10; h++) {
                if (h > max_level) max_level = h;
                
                page * parent = p[h];
                write_parent(parent, child_off, key_to_promote);
                
                // internal page가 꽉 찼는지 확인
                if (parent->num_of_keys == INTERNAL_MAX) {
                    off_t pco = npo[h]; //pco = parent current offset
                    off_t pno = new_page(); //pno = parent next offset
                    
                    // 자식들의 parent page offset update
                    update_children_parent(parent, pco);
                    pwrite(fd, parent, sizeof(page), pco);
                    
                    // 다음 level을 위한 key와 offset 설정
                    key_to_promote = *(int64_t *)parent->reserved; // 저장해둔 대표 키
                    child_off = pco;

                    // reset parent
                    memset(parent, 0, sizeof(page));
                    npo[h] = pno;
                    
                } else {
                    break; // 부모 안 찼으면 중단
                }
            }
        }
    }
    
    // 남은 page 처리
    int64_t key_to_promote = 0;
    off_t child_off = 0;
    
    // leaf flush
    if (p[0]->num_of_keys > 0) {
        off_t current_off = npo[0];
        p[0]->next_offset = 0; // rightmost leaf (끝)
        pwrite(fd, p[0], sizeof(page), current_off);
        
        key_to_promote = p[0]->records[0].key;
        child_off = current_off;
    }
    
    // internal flush
    for (int h = 1; h <= max_level; h++) {
        page *parent = p[h];
        
        // 하위 level에서 올라온 마지막 자식 등록
        if (child_off != 0) {
            write_parent(parent, child_off, key_to_promote);
        }
        
        // internal page에 내용이 있거나, 자식이 하나라도 있다면(p0) 저장
        if (parent->num_of_keys > 0 || parent->next_offset != 0) {
            off_t current_off = npo[h];
            
            update_children_parent(parent, current_off);
            pwrite(fd, parent, sizeof(page), current_off);
            
            key_to_promote = * (int64_t * )parent->reserved;
            child_off = current_off;
        } else {
            // 더 이상 부모가 없으면(현재 level이 비었음), 이전 level의 child_off가 루트임
            hp->rpo = child_off;
        }
    }
    
    // 최종 root 설정
    hp->rpo = child_off;
    
    if (hp->rpo != 0) {
        page *root = load_page(hp->rpo);
        root->parent_page_offset = 0;
        pwrite(fd, root, sizeof(page), hp->rpo);
        
        // 기존 rt 해제 및 새 root caching
        if (rt != NULL) free(rt);
        rt = root;
    } else {
        // 데이터가 하나도 없었던 경우
        if (rt != NULL) { free(rt); rt = NULL; }
    }
    
    pwrite(fd, hp, sizeof(H_P), 0);
    free(hp);
    hp = load_header(0);
    
    close(temp_record);
    unlink("temp_record.dat");
    for(int i=0; i<10; i++) free(p[i]);
    return;
}
//fin

// // 재귀적으로 노드와 엣지를 출력하는 헬퍼 함수
// void print_dot_node(off_t p_off, FILE *fp) {
//     page *p = load_page(p_off);
    
//     // 1. 노드 정의 (Node Definition)
//     fprintf(fp, "node%ld [label=\"", p_off);
//     if (p->is_leaf) {
//         // Leaf Node: Keys 출력
//         for(int i=0; i<p->num_of_keys; i++) {
//             if(i>0) fprintf(fp, " | ");
//             fprintf(fp, "<f%d> %ld", i, p->records[i].key);
//         }
//     } else {
//         // Internal Node: Keys 출력
//         fprintf(fp, "<p0> *"); // P0 포인터
//         for(int i=0; i<p->num_of_keys; i++) {
//             fprintf(fp, " | %ld | <p%d> *", p->b_f[i].key, i+1);
//         }
//     }
//     fprintf(fp, "\"];\n");
    
//     // 2. 엣지 연결 (Edge Definition) - Internal Node만 자식이 있음
//     if (!p->is_leaf) {
//         // P0 (가장 왼쪽 자식)
//         fprintf(fp, "\"node%ld\":p0 -> \"node%ld\";\n", p_off, p->next_offset);
//         print_dot_node(p->next_offset, fp); // 재귀 호출
        
//         // 나머지 자식들
//         for(int i=0; i<p->num_of_keys; i++) {
//             fprintf(fp, "\"node%ld\":p%d -> \"node%ld\";\n", p_off, i+1, p->b_f[i].p_offset);
//             print_dot_node(p->b_f[i].p_offset, fp); // 재귀 호출
//         }
//     }
    
//     free_page(p);
// }

// // 메인 호출 함수
// void draw_tree() {
//     if (hp->rpo == 0) return;
    
//     FILE *fp = fopen("tree.dot", "w");
//     fprintf(fp, "digraph g {\n");
//     fprintf(fp, "node [shape = record, height = 1.];\n"); // 노드 모양 설정
    
//     print_dot_node(hp->rpo, fp);
    
//     fprintf(fp, "}\n");
//     fclose(fp);
//     printf("DOT file generated. Run: dot -Tpng tree.dot -o tree.png\n");
// }






