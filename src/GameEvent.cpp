#include "GameEvent.h"

void GameEvent::setData(void *data)
{
    if(this->data.size() > 0){
        this->data.clear();
        read_position = 0;
    }
    memcpy(&type, data, sizeof(int));
    this->data.insert(this->data.end(), (char*)data, (char*)data + 256);
    read_position += sizeof(int); // skip the event type portion
}

const char *GameEvent::getData()    {return &data[0];}


// WRITE
void GameEvent::setType(int type)
{
    if(data.size() > 0){
        data.clear();
        read_position = 0;
    }

    data.insert(data.end(), (char*)&type, (char*)&type + sizeof(int));
    this->type = type;
}

void GameEvent::pushData(int i){
    data.insert(data.end(), (char*)&i, (char*)&i + sizeof(int));
}

void GameEvent::pushData(float f){
    data.insert(data.end(), (char*)&f, (char*)&f + sizeof(float));
}

void GameEvent::pushData(bool b){
    data.insert(data.end(), (char*)&b, (char*)&b + sizeof(bool));
}

void GameEvent::pushData(void *data, unsigned int data_size){
    this->data.insert(this->data.end(), (char*)data, (char*)data + data_size);
}

// READ
void GameEvent::readData(void *dst, unsigned int data_size)
{
    if(data.size() == 0){
        std::cout << "Error while reading event: There is no data to read \n";
        return;
    }
    memcpy(dst,&data[read_position], data_size);
    read_position += data_size;
}

const char *GameEvent::readData(int data_size)
{
    char *buffer = &data[read_position];
    read_position += data_size;
    return buffer;
}

std::string GameEvent::readString(unsigned int string_size)
{
    
    char* str_buffer = (char*)malloc(string_size+1);
    str_buffer[string_size] = '\0';

    std::string str = str_buffer;

    free(str_buffer);
    read_position += string_size;

    return str;
}

int GameEvent::readInt()
{
    int i;
    memcpy(&i, &data[read_position], sizeof(int));
    read_position += sizeof(int);
    return i;
}
