/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height){
     int index = get_global_id(0);
    
     int x = index % ((width + 1)/n_width);
     int y = index / ((width + 1)/n_height);

     x = (x * n_width) + iteration%n_width;
     y = (y * n_height) + iteration%n_height;

     int index1 = y * width + x;
     int index2 = y * width + (x+1) % width;
     int index3 = (y+1) * width + x;
     int index4 = (y+1) * width + (x+1) % width;

     if (y < height-1) {
         if (data[index1] == 'S' && data[index3] != 'S') {
             char tmp = data[index1];
             data[index1] = data[index3];
             data[index3] = tmp;
         }
         if (data[index2] == 'S' && data[index4] != 'S') {
             char tmp = data[index2];
             data[index2] = data[index4];
             data[index4] = tmp;
         }
         // If index2 has wrapped around, don't make this move
         if (index1 < index2) {
             if (data[index1] == 'S' && data[index3] == 'S'&& data[index4] != '@') {
                 char tmp = data[index1];
                 data[index1] = data[index4];
                 data[index4] = tmp;
             }
         }
         // If index1 has wrapped around, don't make this move
         if (index2 > index1) {
             if (data[index2] == 'S' && data[index4] == 'S'&& data[index3] != '@') {
                 char tmp = data[index3];
                 data[index3] = data[index2];
                 data[index2] = tmp;
             }
         }
     }
 }
