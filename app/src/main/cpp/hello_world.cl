/**
 * This kernel function only fills a buffer with the sentence 'Hello World!'.
 **/

 __kernel void helloWorld(__global char* data,int iteration, 
        int width, int height, int n_width, int n_height){
     int index = get_global_id(0);

     int x = index % ((width + 1)/n_width);
     int y = index / ((height + 1)/n_height);

     x = (x * n_width);
     y = (y * n_height);

     index = y * width + x;
     data[index] = iteration + x + y;
     data[index+1] = iteration + x + y;
     data[index+width] = iteration + x + y;
     data[index+width+1] = iteration + x + y;
 }
