#pragma once
#include <string>
#include <exception>
#include <stdexcept>

namespace impl{

    struct string_long {
        char *m_buffer_ptr; // 8 bytes
        size_t m_size;      // 8 bytes
        size_t m_capacity;  // 8 bytes
    };

    struct string_short {
        static constexpr size_t capacity{23};
        char m_buffer[capacity]; // 23 bytes
        unsigned char m_size;    // 1 byte
    };

    class string {
        using iterator = char*;
        using const_iterator = const char*;

    private:
        union {
            string_short s;
            string_long l;
        } m_data;

        bool is_long() const {
            return (m_data.s.m_size & 0x80) != 0;
        }
        

    public:
        // ---------------------------- constructors -------------------------

        // default constructor
        string(){
            m_data.s.m_size = 0;
        }

        // from literal constructor
        string(const char* chars_array){
            size_t length{};

            while(chars_array[length] != '\0'){
                ++length;
            }
            if(length < 23){
                m_data.s.m_size = length;
                for(size_t i{}; i < length; ++i){
                    m_data.s.m_buffer[i] = chars_array[i];
                }
                m_data.s.m_buffer[length] = '\0';
            }else{
                m_data.l.m_size = length;
                m_data.l.m_capacity = length;
                m_data.l.m_buffer_ptr = new char[length + 1];
                m_data.l.m_capacity |= (1ULL << 63);

                for(size_t i{}; i< length; ++i){
                    m_data.l.m_buffer_ptr[i] = chars_array[i];
                }
                m_data.l.m_buffer_ptr[length] = '\0';
            }
        }

        // copy constructor
        string(const string& other){
            if(other.is_long()){
                m_data.l.m_size = other.size();
                m_data.l.m_capacity = other.m_data.l.m_capacity;
                m_data.l.m_buffer_ptr = new char[other.size() + 1];
                m_data.l.m_capacity |= (1ULL << 63);

                for(size_t i{}; i < other.size(); ++i){
                    m_data.l.m_buffer_ptr[i] = other.m_data.l.m_buffer_ptr[i];
                }
                m_data.l.m_buffer_ptr[m_data.l.m_size] = '\0';

            }else{
                m_data.s.m_size = other.m_data.s.m_size;
                for(size_t i{}; i < other.size(); ++i){
                    m_data.s.m_buffer[i] = other.m_data.s.m_buffer[i];
                }
                m_data.s.m_buffer[m_data.s.m_size] = '\0';

            }
        }

        // move constructor
        string(string&& other){
            std::memcpy(&m_data, &other.m_data, sizeof(other.m_data));
            if(other.is_long()){
                other.m_data.l.m_size = 0;
                other.m_data.l.m_capacity = 0;
                other.m_data.l.m_buffer_ptr = nullptr;
            }else{
                other.m_data.s.m_size = 0;
                other.m_data.s.m_buffer[0] = '\0';
            }
        }

        // swap
        void swap(string& other){
            std::swap(this -> m_data.l.m_buffer_ptr, other.m_data.l.m_buffer_ptr);
            std::swap(this -> m_data.l.m_size, other.m_data.l.m_size);
            std::swap(this -> m_data.l.m_capacity, other.m_data.l.m_capacity);
        }
        friend void swap(string& lhs, string& rhs){
            lhs.swap(rhs);
        }

        // copy assignment
        string& operator=(string other){
            this -> swap(other);
            return *this;
        }

        // construct a string from an arbitrary range [first, last)
        template<typename InputIt>
        string(InputIt first, InputIt last){
            std::span<const char> view{first, last};
            size_t length = view.size();
            const char* raw_ptr = view.data();

            if(length < 23){
                m_data.s.m_size = static_cast<unsigned char>(length);
                if(length >0){
                    std::memcpy(m_data.s.m_buffer, raw_ptr, length);
                }
                m_data.s.m_buffer[length] = '\0';
            }else{
                m_data.l.m_size = length;
                m_data.l.m_capacity = length;
                m_data.l.m_capacity |= (1Ull << 63);
                m_data.l.m_buffer_ptr = new char[length + 1];
                std::memcpy(m_data.l.m_buffer_ptr, raw_ptr, length);
                m_data.l.m_buffer_ptr[length] = '\0';
            }
        }

        // construct a string from an initializer list
        string(std::initializer_list<char> list){
            size_t length = list.size();

            if(length < 23){
                m_data.s.m_size = static_cast<unsigned char>(length);
                if(length >0){
                    std::memcpy(m_data.s.m_buffer, list.begin(), length);
                }
                m_data.s.m_buffer[length] = '\0';
            }else{
                m_data.l.m_size = length;
                m_data.l.m_capacity = length;
                m_data.l.m_capacity |= (1Ull << 63);
                m_data.l.m_buffer_ptr = new char[length + 1];
                std::memcpy(m_data.l.m_buffer_ptr, list.begin(), length);
                m_data.l.m_buffer_ptr[length] = '\0';
            }
        }

        // constructs a string with count copies of the character ch
        string(size_t count, char ch){
            if(count < 23){
                m_data.s.m_size = static_cast<unsigned char>(count);
                if(count >0){
                    for(size_t i{}; i < count; ++i){
                        m_data.s.m_buffer[i] = ch;
                    }
                }
                m_data.s.m_buffer[count] = '\0';
            }else{
                m_data.l.m_size = count;
                m_data.l.m_capacity = count;
                m_data.l.m_capacity |= (1Ull << 63);
                m_data.l.m_buffer_ptr = new char[count + 1];
                for(size_t i{}; i < count; ++i){
                    m_data.l.m_buffer_ptr[i] = ch;
                }

                m_data.l.m_buffer_ptr[count] = '\0';
            }
        }

        // destructor
        ~string(){
            if(is_long()){
                delete[] m_data.l.m_buffer_ptr;
            }
        }

        // ---------------------- capacity functions -------------------------
        // size of the string
        size_t size(){
            if(is_long()){
                return m_data.l.m_size;
            }else{
                return m_data.s.m_size & 0x7f;
            }
        }

        // const version
        size_t size() const{
            if(is_long()){
                return m_data.l.m_size;
            }else{
                return m_data.s.m_size & 0x7f;
            }
        }

        // capacity
        size_t capacity(){
            if(is_long()){
                return m_data.l.m_capacity & ~(1ULL << 63);
            }else{
                return 22;
            }
        }

        // checks if string is empty
        bool empty(){
            if(is_long()){
                return m_data.l.m_size == 0;
            }else{
                return m_data.s.m_size == 0;
            }
        }

        // allocate new storage
        void reserve(size_t new_capacity){
            if(new_capacity <= capacity()){
                return;
            }
            if(is_long()){
                char* temp_ptr = new char[new_capacity + 1];
                std::memcpy(temp_ptr, m_data.l.m_buffer_ptr, m_data.l.m_size);
                temp_ptr[m_data.l.m_size] = '\0';

                delete[] m_data.l.m_buffer_ptr;
                m_data.l.m_buffer_ptr = temp_ptr;
                m_data.l.m_capacity = new_capacity;
                m_data.l.m_capacity |= (1Ull << 63); 

            }
            else{
                size_t curr_size = size();
                char * temp_ptr = new char[new_capacity + 1];
                std::memcpy(temp_ptr, m_data.s.m_buffer, curr_size);
                temp_ptr[curr_size] = '\0';

                m_data.l.m_buffer_ptr = temp_ptr;
                m_data.l.m_size = curr_size;
                m_data.l.m_capacity = new_capacity;
                m_data.l.m_capacity |= (1ULL << 63);
            }
        }

        // ---------------------------- Element Access ------------------------

        // data() return the pointer to the first character of the string
        char* data(){
            if(is_long()){
                return m_data.l.m_buffer_ptr;
            }else{
                return m_data.s.m_buffer;
            }
        }

        // const version
        const char* data() const{
            if(is_long()){
                return m_data.l.m_buffer_ptr;
            }else{
                return m_data.s.m_buffer;
            }
        }

        // Array subscript operator[]
        char& operator[](size_t pos){
            if(is_long()){
                return m_data.l.m_buffer_ptr[pos];
            }else{
                return m_data.s.m_buffer[pos];
            }
        }

        // const version
        const char& operator[](size_t pos) const{
            if(is_long()){
                return m_data.l.m_buffer_ptr[pos];
            }else{
                return m_data.s.m_buffer[pos];
            }
        }

        // indexed access with bounds checking
        char& at(size_t pos){
            if(is_long()){
                if(pos < size()) return m_data.l.m_buffer_ptr[pos];
                else throw std::out_of_range("Invalid index");
            }else{
                if(pos < size()) return m_data.s.m_buffer[pos];
                else throw std::out_of_range("Invalid index");
            }
        }

        // Returns a reference to the first character of the string
        char& front(){
            if(is_long()){
                return *m_data.l.m_buffer_ptr;
            }else{
                return *m_data.s.m_buffer;
            }            
        }

        // Returns a reference to the last character of the string
        char& back(){
            if(is_long()){
                return m_data.l.m_buffer_ptr[m_data.l.m_size - 1];
            }else{
                return m_data.s.m_buffer[m_data.s.m_size - 1];
            }            
        }

        // ---------------------------------- Iterators -----------------------------

        char* begin(){
            return this -> data();             
        }
        const char* begin() const{
            return this -> data();
        }
        char* end(){
            return data() + size();           
        }
        const char* end() const{
            return data() + size();            
        }

        // ------------------------------- Modifiers -----------------------------

        // Appends the given character ch to the end of the string
        void push_back(char ch){
            if (is_long()) {
                size_t cap = capacity();
                if (m_data.l.m_size >= cap) {
                    reserve(cap == 0 ? 32 : cap * 2);
                }
                m_data.l.m_buffer_ptr[m_data.l.m_size] = ch;
                m_data.l.m_size++;
                m_data.l.m_buffer_ptr[m_data.l.m_size] = '\0';
            } else {
                size_t curr_size = size();
                if (curr_size < 22) {
                    m_data.s.m_buffer[curr_size] = ch;
                    m_data.s.m_size++;
                    m_data.s.m_buffer[curr_size + 1] = '\0';
                } else {
                    size_t new_cap = 44;
                    char* temp_ptr = new char[new_cap + 1];
                    std::memcpy(temp_ptr, m_data.s.m_buffer, curr_size);
                    temp_ptr[curr_size] = ch;
                    temp_ptr[curr_size + 1] = '\0';
                    m_data.l.m_buffer_ptr = temp_ptr;
                    m_data.l.m_size = curr_size + 1;
                    m_data.l.m_capacity = new_cap;
                    m_data.l.m_capacity |= (1ULL << 63);
                }
            }
        }

        // removes a character from the end of the string
        void pop_back(){
            if (empty()) {
                return;
            }
            if (is_long()) {
                m_data.l.m_size--;
                m_data.l.m_buffer_ptr[m_data.l.m_size] = '\0';
            } else {
                m_data.s.m_size--;
                m_data.s.m_buffer[m_data.s.m_size] = '\0';
            }
        }

        // inserts the characters from the range [first, last)
        // before the element (if any) pointed to by pos
        template<typename InputIt>
        void insert(const_iterator pos, InputIt first, InputIt last){
            std::span<const char> view{first, last};
            size_t count = view.size();
            if (count == 0) return;

            size_t offset = pos - data();
            size_t current_size = size();
            size_t new_size = current_size + count;

            if (new_size > capacity()) {
                size_t cap = capacity();
                size_t new_cap = cap * 2;
                if (new_cap < new_size) new_cap = new_size;
                if (new_cap < 44) new_cap = 44;
                reserve(new_cap);
            }

            char* base = data();
            std::memmove(base + offset + count, base + offset, current_size - offset);
            std::memcpy(base + offset, view.data(), count);

            if (is_long()) {
                m_data.l.m_size = new_size;
            } else {
                m_data.s.m_size = static_cast<unsigned char>(new_size);
            }
            base[new_size] = '\0';
        }

        // Removes the characters in the range [first, last)
        iterator erase(const_iterator first, const_iterator last){
            size_t offset = first - data();
            size_t num_to_erase = last - first;
            if (num_to_erase > 0) {
                size_t num_after = end() - last;
                std::memmove(data() + offset, data() + (last - data()), num_after);
                if (is_long()) {
                    m_data.l.m_size -= num_to_erase;
                } else {
                    m_data.s.m_size -= num_to_erase;
                }
                data()[size()] = '\0';
            }
        return data() + offset;
        }
    };

} 