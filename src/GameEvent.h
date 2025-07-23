#include<iostream>
#include <sstream>
#include <vector>;

class GameEvent{
private:
    int read_position = 0;
    std::vector<char> data;

    int type = -1;
public:
    GameEvent(){}
    inline GameEvent(int type){setType(type);}
    inline GameEvent(void* data){setData(data);}

    inline int getType(){return type;}

    void setData(void* data);
    const char* getData();
    
    /**
     * The event type must be set before any other function.
     * Setting a new type, clears all the data stored before.
     * 
     * This is done this way to facilitate reusability.
     */
    void setType(int type);

    //TODO: explain these funtions uses.

    void pushData(int i);
    void pushData(float f);
    void pushData(bool b);
    void pushData(void* data, unsigned int data_size);

    void readData(void* dst, unsigned int data_size);
    const char* readData(int data_size);

    std::string readString(unsigned int string_size);
    int readInt();
};