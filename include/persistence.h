#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include "stdio.h"

template<typename T>
struct persist_t {
    int id;
    T persist_struct;
};

template<typename T>
struct persists_t {
    int size;
    persist_t<T> *persist_array;
};

template<class retT>
class Persistence
{
public:
    Persistence(int id);

    ~Persistence();

    static void set_location(std::string location);

    int get_id(void);

    std::string get_string_id(void);

    int read_persists(persists_t<retT> **persists);

    int write_persists(persists_t<retT> *persists);

    int get_persist(persist_t<retT> **persist, int entry_id);

    int get_persist(retT **persist_struct, int entry_id);

    int remove_persist(int entry_id);

    int add_persist(persist_t<retT> *persist);

    int add_persist(retT *persist_struct, int entry_id);

private:
    bool _entry_exists(persists_t<retT> *persists, int entry_id);
    int _id;
    static std::string _location;
};

template<class retT>
Persistence<retT>::Persistence(int id)
{
    _id = id;
}

template<class retT>
Persistence<retT>::~Persistence() {}

template<class retT>
void Persistence<retT>::set_location(std::string location) {
    Persistence<retT>::_location = std::string(location);
}

template<class retT>
int Persistence<retT>::get_id(void) {
    return _id;
}

template<class retT>
std::string Persistence<retT>::get_string_id(void) {
    int length = std::snprintf( nullptr, 0, "%i", _id );
    assert( length >= 0 );

    char* buf = new char[length + 1];
    std::snprintf( buf, length + 1, "%i", _id );

    std::string str( buf );
    delete[] buf;
    return str;
}

template<class retT>
int Persistence<retT>::read_persists(persists_t<retT> **persists)
{
    int size = 0;
    std::string file_location = Persistence<retT>::_location + get_string_id();
    FILE *fp = fopen(file_location.c_str(), "r");
    if(!fp) {
        *persists = new persists_t<retT>({.size=0, .persist_array=nullptr});
        return 0;
    }
    fseek(fp, 0, SEEK_SET);
    fread(&size, sizeof(int), 1, fp);
    if(size>0) {
        fseek(fp, sizeof(int), SEEK_SET);
        persist_t<retT> *persist_array = new persist_t<retT>[size];
        fread(persist_array, sizeof(persist_t<retT>)*size, 1, fp);
        *persists = new persists_t<retT>({.size=size, .persist_array=persist_array});
    } else {
        *persists = new persists_t<retT>({.size=size, .persist_array=nullptr});
    }
    fclose(fp);
    return 0;
}

template<class retT>
int Persistence<retT>::write_persists(persists_t<retT> *persists)
{
    std::string file_location = Persistence<retT>::_location + get_string_id();
    FILE *fp = fopen(file_location.c_str(), "w");
    if(!fp) {
        return -1;
    }
    fseek(fp, 0, SEEK_SET);
    fwrite(&(persists->size), sizeof(int), 1, fp);
    fseek(fp, sizeof(int), SEEK_SET);
    fwrite(persists->persist_array, sizeof(persist_t<retT>)*persists->size, 1, fp);
    fclose(fp);
    return 0;
}

template<class retT>
int Persistence<retT>::get_persist(persist_t<retT> **persist, int entry_id)
{
    persists_t<retT> *persists;
    int err = read_persists(&persists);
    if(err) {
        return err;
    }
    for (int i=0; i<persists->size; i++) {
        if((persists->persist_array)[i].id == entry_id) {
            *persist = new persist_t<retT>((persists->persist_array)[i]);
            break;
        }
    }
    delete persists->persist_array;
    delete persists;
    return 0;
}

template<class retT>
int Persistence<retT>::get_persist(retT **persist_struct, int entry_id)
{
    persist_t<retT> *persist;
    int err = get_persist(&persist, entry_id);
    if(err) {
        return err;
    }
    if(persist != nullptr) {
        *persist_struct = new retT(persist->persist_struct);
        delete persist;
    }
    return 0;
}

template<class retT>
int Persistence<retT>::remove_persist(int entry_id)
{
    persists_t<retT> *persists;
    int err = read_persists(&persists);
    if(err) {
        return err;
    }
    int new_size=0;
    if(_entry_exists(persists, entry_id)) {
        new_size = persists->size - 1;
    } else {
        return 0;
    }
    persist_t<retT> *persist_array = new persist_t<retT>[new_size];
    for (int i=0; i<persists->size; i++) {
        if((persists->persist_array)[i].id == entry_id) {

            continue;
        }
        memcpy(&persist_array[i], &(persists->persist_array)[i], sizeof(persist_t<retT>));
    }
    delete persists->persist_array;
    delete persists;
    persists = new persists_t<retT>({.size=new_size, .persist_array=persist_array});
    err = write_persists(persists);
    delete persists->persist_array;
    delete persists;
    return err;
}

template<class retT>
int Persistence<retT>::add_persist(persist_t<retT> *persist)
{
    persists_t<retT> *persists;
    int err = read_persists(&persists);
    if(err) {
        return err;
    }
    int size=0;
    if(_entry_exists(persists, persist->id)) {
        size = persists->size;
    } else {
        size = persists->size + 1;
    }
    persist_t<retT> *persist_array = new persist_t<retT>[size];
    for (int i=0; i<persists->size;i++) {
        if((persists->persist_array)[i].id == persist->id) {
            continue;
        }
        memcpy(&persist_array[i], &(persists->persist_array)[i], sizeof(persist_t<retT>));
    }
    memcpy(&persist_array[size-1], persist, sizeof(persist_t<retT>));
    delete persists->persist_array;
    delete persists;
    persists = new persists_t<retT>({.size=size, .persist_array=persist_array});
    err = write_persists(persists);
    delete persists->persist_array;
    delete persists;
    return err;
}

template<class retT>
int Persistence<retT>::add_persist(retT *persist_struct, int entry_id)
{
    persist_t<retT> persist({.id=entry_id, .persist_struct=*persist_struct});
    return add_persist(&persist);
}

template<class retT>
bool Persistence<retT>::_entry_exists(persists_t<retT> *persists, int entry_id)
{
    for (int i=0; i<persists->size;i++) {
        if((persists->persist_array)[i].id == entry_id) {
            return true;
        }
    }
    return false;
}

template<class retT> 
std::string Persistence<retT>::_location = "";

#endif
