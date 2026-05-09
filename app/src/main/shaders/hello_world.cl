/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

bool is_powder(char material) {
    switch(material) {
        case 'S':
        case 'W':
            return true;
    }
    return false;
}
bool is_liquid(char material) {
    switch(material) {
        case 'S':
            return false;
        case 'W':
            return true;
    }
    return false;
}

__kernel void helloWorld(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height){

    int index = get_global_id(0);

    int x = index % ((width + 1)/n_width);
    int y = index / ((width + 1)/n_height);

    x = (x * n_width) + (iteration%n_width);
    y = (y * n_height) + (iteration%n_height);

    int index1 = y * width + x;
    int index2 = y * width + (x+1) % width;
    int index3 = (((y+1)%height) * width) + x;
    int index4 = (y+1) * width + (x+1) % width;
    
    bool moved = false;
    if (index1 < index3) {
        if (data[index1] != ' ' && data[index3] == ' ') {
            char tmp = data[index1];
            data[index1] = data[index3];
            data[index3] = tmp;
            moved = true;
        }
        if (data[index2] != ' ' && data[index4] == ' ') {
            char tmp = data[index2];
            data[index2] = data[index4];
            data[index4] = tmp;
            moved = true;
        }
        if (moved) return;
        // If index2 has wrapped around, don't make this move
        if (index1 >= index2) return;

        
        if (is_powder(data[index1]) && data[index1] != ' ' && data[index3] != ' '&& data[index4] == ' ') {
            char tmp = data[index1];
            data[index1] = data[index4];
            data[index4] = tmp;
            moved = true;
        }
        if (is_powder(data[index2]) && data[index2] != ' ' && data[index4] != ' '&& data[index3] == ' ') {
            char tmp = data[index3];
            data[index3] = data[index2];
            data[index2] = tmp;
            moved = true;
        }
        if (moved) return;
        if (is_liquid(data[index1]) && data[index1] != ' ' && data[index2] == ' ' ||
            is_liquid(data[index2]) && data[index2] != ' ' && data[index1] == ' ') {
            char tmp = data[index1];
            data[index1] = data[index2];
            data[index2] = tmp;
        }
        if (is_liquid(data[index3]) && data[index3] != ' ' && data[index4] == ' ' ||
            is_liquid(data[index4]) && data[index4] != ' ' && data[index3] == ' ') {
            char tmp = data[index3];
            data[index3] = data[index4];
            data[index4] = tmp;
        }
    }
}
