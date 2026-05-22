#include "chunk_manager.h"
#include "buffer.h"
#include "chunk.h"

ChunkManager::ChunkManager(int n_world_width, int n_world_height, CL* cl) :
    world_width(n_world_width), 
    world_height(n_world_height)
{
    camera = {0,0,1};

    chunk_size = 64;

    render_buffer = init_render_buf(world_width,world_height,'?');

    chunks = std::vector<std::vector<Chunk>>();

    for (int y = 0; y < world_height; y+= chunk_size) {
        std::vector<Chunk> row = std::vector<Chunk>();
        for (int x = 0; x < world_width; x+= chunk_size) {
            Chunk n_chunk(x / chunk_size, y / chunk_size,chunk_size,cl);
            // if(y + chunk_size >= world_height) {
            //     for (int i = 0; i < chunk_size; i++) {
            //         n_chunk.set_cell(i,(chunk_size-5),'R');
            //     }
            // }

            row.push_back(
                n_chunk
            );
        }
        chunks.push_back(row);
    }

    // Tell the kernel what size each chunk is for rendering
    cl->setArg(3,chunk_size,cl->render_kernel);
    cl->setArg(4,chunk_size,cl->process_kernel);

    // Create the GPU buffer for rendering and link it to the render kernel
    render_buffer_index = cl->makeRenderBuffer(render_buffer);
    cl->setArg(8,render_buffer,render_buffer_index,cl->render_kernel);
    // Link the render buffer to the process kernel too
    // cl->setArg(0, render_buffer,render_buffer_index,cl->process_kernel);
    
    // Buffer where (0,0),(0,1),(0,2),(0,3) = TL, TR, BL, BR diagonal chunks,
    // (1->chunk_size,0) = North, 
    // (1->chunk_size,1) = East,
    // (1->chunk_size,2) = South,
    // (1->chunk_size,3) = West,
    swap_requests = init_buf(chunk_size,4,0);
    swap_requests_index = cl->makeBuffer(swap_requests);
    // cl->setArg(9,swap_requests,swap_requests_index,cl->process_kernel);
}

Chunk* ChunkManager::get_chunk(int x, int y) {
    if (y >= chunks.size() || y < 0 || x >= chunks.at(y).size() || x < 0) { return nullptr; }
    return &chunks.at(y).at(x);
}

buffer ChunkManager::get_render_buffer() {
    return render_buffer;
}

void ChunkManager::render_chunk(CL* cl,Chunk* chunk) {
    // Tell the kernel the chunk's data and the chunk's position
    cl->setArg(0,chunk->get_data(),chunk->buffer_index,cl->render_kernel);
    cl->setArg(1,chunk->chunk_x,cl->render_kernel);
    cl->setArg(2,chunk->chunk_y,cl->render_kernel);

    cl->setArg(4,camera.x,cl->render_kernel);
    cl->setArg(5,camera.y,cl->render_kernel);

    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk->get_data().data, chunk->buffer_index);
    cl->enqueueKernel(chunk_size,chunk_size,cl->render_kernel);
    cl->enqueueRenderReadBuffer(world_width,world_height, render_buffer.data, render_buffer_index);
}

void ChunkManager::update_chunks(CL* cl) {
    for (int y = 0; y < chunks.size(); y++) {
        for (int x = 0; x < chunks.at(y).size(); x++) {
        
        Chunk* chunk = get_chunk(x,y);

        char materials[] = {'S','W','R',' '};

        if (y == 0&&x==0 && chunk->get_iteration() % 3 == 0 && chunk->get_update_count() < 3000) {
            chunk->set_cell(chunk_size / 2,chunk_size / 4, 'S');
        }
        refresh_chunk(cl,chunk);
        
        cl->setArg(0,chunk->get_data(),chunk->buffer_index,cl->process_kernel);
        cl->setArg(2,chunk->chunk_x,cl->process_kernel);
        cl->setArg(3,chunk->chunk_y,cl->process_kernel);

        for (int j = 0; j < chunk->highest_speed; j++) {
            // Create chunk_size/2 x chunk_size/2 tasks, each processing a 2x2 block.
            cl->setArg(1,chunk->get_iteration(),cl->process_kernel);
            cl->setArg(9,chunk->get_update_count_buf(),chunk->to_update_count_index,cl->process_kernel);
            cl->enqueueKernel(chunk->get_update_count(), 1, cl->process_kernel);

            // Render read buffer because this is exactly 4 bytes (1 int) rather than regular data 
            // (which contain x,y,material etc, so many bytes per entry)
            cl->enqueueRenderReadBuffer(4,1, chunk->get_update_count_buf().data, chunk->to_update_count_index);
            
            chunk->iterate();

            // process_swap_requests(cl,chunk);

        }

        cl->enqueueReadBuffer(chunk_size,chunk_size, chunk->get_data().data, chunk->buffer_index);
        render_chunk(cl,chunk);
        }
    }
}


void ChunkManager::refresh_chunk(CL* cl,Chunk* chunk) {
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk->get_data().data, chunk->buffer_index);
    cl->enqueueRenderWriteBuffer(4, 1, chunk->get_update_count_buf().data, chunk->to_update_count_index);
}

void ChunkManager::input(InputState input_state) {
    if (input_state.camera_up)   {camera.y  += camera.speed; }
    if (input_state.camera_right){camera.x  += camera.speed; }
    if (input_state.camera_down) {camera.y  -= camera.speed; }
    if (input_state.camera_left) {camera.x  -= camera.speed; }
}

void ChunkManager::process_swap_requests(CL* cl,Chunk* chunk) {
    cl->enqueueReadBuffer(chunk_size,4, swap_requests.data, swap_requests_index);
    cl->enqueueReadBuffer(chunk_size,chunk_size, chunk->get_data().data, chunk->buffer_index);

    // buffer_value tl = get_buffer(swap_requests,0,0);
    // buffer_value tr = get_buffer(swap_requests,0,1);
    // buffer_value bl = get_buffer(swap_requests,0,2);
    // buffer_value br = get_buffer(swap_requests,0,3);
    
    // Only 4 neighbours, so store in an array
    Chunk** updated_neighbours = new Chunk*[4] {nullptr,nullptr,nullptr,nullptr};//std::vector<Chunk*>(4);

    int side;

    int skip = 0;

    side = 0;
    for (int pos = 0; pos < swap_requests.width; pos++) {
        buffer_value bv = get_buffer(swap_requests, pos, side);
        if (bv.material != ' ' && bv.material != 0) {
            // std::cout << "Swap request for [" << bv.material << "] (" << bv.x << "," << bv.y << ")" << " At: " << pos << "," << side << std::endl;
            int x = chunk->chunk_x;
            int y = chunk->chunk_y;
            switch(side) {
                case 0: y -= 1; break;
                case 1: x += 1; break;
                case 2: y += 1; break;
                case 3: x -= 1; break;
            }

            Chunk* neighbour = get_chunk(x,y);
            if (neighbour) {
                // std::cout << chunk-> chunk_x << "," << chunk->chunk_y << " -> " <<neighbour->chunk_x << "," << neighbour->chunk_y << std::endl;
                buffer_value nv = {(char)pos,(char)(chunk_size-1-skip),'?'};
                nv.material = get_buffer(neighbour->get_data(),pos,chunk_size-1-skip).material;
                set_buffer(chunk->get_data(),pos,skip,nv);
                bv.x = pos;
                bv.y = chunk_size-1-skip;
                set_buffer(neighbour->get_data(),pos,chunk_size-1-skip,bv);
                updated_neighbours[side] = neighbour;
            }
        }
    }
    side = 1;
    for (int pos = 0; pos < swap_requests.width; pos++) {
        buffer_value bv = get_buffer(swap_requests, pos, side);
        if (bv.material != ' ' && bv.material != 0) {
            // std::cout << "Swap request for [" << bv.material << "] (" << bv.x << "," << bv.y << ")" << " At: " << pos << "," << side << std::endl;
            int x = chunk->chunk_x;
            int y = chunk->chunk_y;
            switch(side) {
                case 0: y -= 1; break;
                case 1: x += 1; break;
                case 2: y += 1; break;
                case 3: x -= 1; break;
            }

            Chunk* neighbour = get_chunk(x,y);
            if (neighbour) {
                // std::cout << chunk-> chunk_x << "," << chunk->chunk_y << " -> " <<neighbour->chunk_x << "," << neighbour->chunk_y << std::endl;
                buffer_value nv = {(char)(skip),(char)pos,'?'};
                nv.material = get_buffer(neighbour->get_data(),chunk_size-1-skip,pos).material;
                set_buffer(chunk->get_data(),chunk_size-1-skip,pos,nv);
                bv.x = skip;
                bv.y = pos;
                set_buffer(neighbour->get_data(),skip,pos,bv);
                updated_neighbours[side] = neighbour;
            }
        }
    }
    side = 2;
    for (int pos = 0; pos < swap_requests.width; pos++) {
        buffer_value bv = get_buffer(swap_requests, pos, side);
        if (bv.material != ' ' && bv.material != 0) {
            // std::cout << "Swap request for [" << bv.material << "] (" << bv.x << "," << bv.y << ")" << " At: " << pos << "," << side << std::endl;
            int x = chunk->chunk_x;
            int y = chunk->chunk_y;
            switch(side) {
                case 0: y -= 1; break;
                case 1: x += 1; break;
                case 2: y += 1; break;
                case 3: x -= 1; break;
            }

            Chunk* neighbour = get_chunk(x,y);
            if (neighbour) {
                // std::cout << chunk-> chunk_x << "," << chunk->chunk_y << " -> " <<neighbour->chunk_x << "," << neighbour->chunk_y << std::endl;
                buffer_value nv = {(char)pos,(char)skip,'?'};
                nv.material = get_buffer(neighbour->get_data(),pos,skip).material;
                set_buffer(chunk->get_data(),pos,chunk_size-1-skip,nv);
                bv.x = pos;
                bv.y = skip;
                set_buffer(neighbour->get_data(),pos,skip,bv);
                updated_neighbours[side] = neighbour;
            }
        }
    }
    side = 3;
    for (int pos = 0; pos < swap_requests.width; pos++) {
        buffer_value bv = get_buffer(swap_requests, pos, side);
        if (bv.material != ' ' && bv.material != 0) {
            // std::cout << "Swap request for [" << bv.material << "] (" << bv.x << "," << bv.y << ")" << " At: " << pos << "," << side << std::endl;
            int x = chunk->chunk_x;
            int y = chunk->chunk_y;
            switch(side) {
                case 0: y -= 1; break;
                case 1: x += 1; break;
                case 2: y += 1; break;
                case 3: x -= 1; break;
            }

            Chunk* neighbour = get_chunk(x,y);
            if (neighbour) {
                // std::cout << chunk-> chunk_x << "," << chunk->chunk_y << " -> " <<neighbour->chunk_x << "," << neighbour->chunk_y << std::endl;
                buffer_value nv = {(char)(chunk_size-1-skip),(char)pos,'?'};
                nv.material = get_buffer(neighbour->get_data(),skip,pos).material;
                set_buffer(chunk->get_data(),skip,pos,nv);
                bv.x = chunk_size-1-skip;
                bv.y = pos;
                set_buffer(neighbour->get_data(),chunk_size-1-skip,pos,bv);
                updated_neighbours[side] = neighbour;
            }
        }
    }

    for (int side = 0; side < 4; side++) {
        for (int pos = 0; pos < swap_requests.width * BUFFER_RUN; pos++) {
            swap_requests.data[(pos + side * swap_requests.width * BUFFER_RUN)] = 0;
        }
    }

    cl->enqueueWriteBuffer(swap_requests.width, swap_requests.height, swap_requests.data, swap_requests_index);
    cl->enqueueWriteBuffer(chunk_size, chunk_size, chunk->get_data().data, chunk->buffer_index);

    for (int i = 0; i < 4; i++) {
        Chunk* neighbour = updated_neighbours[i];
        if (neighbour) {
            cl->enqueueWriteBuffer(chunk_size, chunk_size, neighbour->get_data().data, neighbour->buffer_index);
        }
    }
}

void ChunkManager::free_chunks() {
    int freed_count = 0;
    for (int y = 0; y < chunks.size(); y++) {
        for (int x = 0; x < chunks.at(y).size(); x++) {
            get_chunk(x,y)->free();
            freed_count++;
        }
    }
    std::cout << "Freed " << freed_count << " chunks" << std::endl;
}
