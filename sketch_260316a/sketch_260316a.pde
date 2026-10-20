int EMPTY = color(0,0,0);
int YELLOW = color(255,255,0);
int BLUE = color(0,255,255);
int TEXT_COLOUR = color(255,255,255);
Vector planetCore;
HashMap<Integer,Float> colourAngles = new HashMap<>();
HashMap<Integer,Float> colourWeights = new HashMap<>();

boolean[] swaps;
Vector[] velocities;
// Add to colour when drawing the pixel
int[] variation;

int pixelCount = 0;

float initSpeed = 5;
float gravity = 600;

float drag = 0.9;

float speedCap = 16;

Vector[][] numbers = new Vector[][] {
  {new Vector(0,0),new Vector(0,1),new Vector(0,2),new Vector(1,2),new Vector(1,1),new Vector(1,0),new Vector(0,0)},
  {new Vector(0,0),new Vector(0,1),new Vector(0,2)},
  {new Vector(0,0),new Vector(1,0),new Vector(1,1),new Vector(0,1),new Vector(0,2),new Vector(1,2)},
  {new Vector(0,0),new Vector(1,0),new Vector(1,1),new Vector(0,1),new Vector(1,1),new Vector(1,2),new Vector(0,2)},
  {new Vector(0,0),new Vector(0,1),new Vector(1,1),new Vector(1,0),new Vector(1,1),new Vector(1,2)},
  {new Vector(0,0),new Vector(1,0),new Vector(0,0),new Vector(0,1),new Vector(1,1),new Vector(1,2),new Vector(0,2)},
  {new Vector(0,0),new Vector(1,0),new Vector(0,0),new Vector(0,1),new Vector(1,1),new Vector(1,2),new Vector(0,2),new Vector(0,1)},
  {new Vector(0,0),new Vector(1,0),new Vector(1,1),new Vector(1,2)},
  {new Vector(0,0),new Vector(0,1),new Vector(1,1),new Vector(0,1),new Vector(0,2),new Vector(1,2),new Vector(1,1),new Vector(1,0),new Vector(0,0)},
  {new Vector(0,0),new Vector(1,0),new Vector(1,1),new Vector(1,0),new Vector(0,0),new Vector(0,1),new Vector(1,1),new Vector(1,2),new Vector(0,2)}
};

void drawDigit(int digit,int x,int y, int size) {
  Vector[] data = numbers[digit];
  for (int ind = 0; ind < data.length-1; ind++) {
    int cx = (int)data[ind].x;
    int cy = (int)data[ind].y;
    int nx = (int)data[ind+1].x;
    int ny = (int)data[ind+1].y;
    int dx = nx - cx;
    int dy = ny - cy;
    for (int i = 0; i < size; i++) {
      spawn(x,y,YELLOW,0,0);
      //setVariation(x,y,color(256,256,256));
      x += dx;
      y += dy;
    }
  }
}

void drawNumber(String number,int x,int y, int size) {
  for (int i = 0; i < number.length(); i++) {
    drawDigit(number.charAt(i) - '0',x + (int)(i * size * 2),y,size);
  }
}

void clearDigit(int digit,int x,int y, int size) {
  Vector[] data = numbers[digit];
  for (int ind = 0; ind < data.length-1; ind++) {
    int cx = (int)data[ind].x;
    int cy = (int)data[ind].y;
    int nx = (int)data[ind+1].x;
    int ny = (int)data[ind+1].y;
    int dx = nx - cx;
    int dy = ny - cy;
    for (int i = 0; i < size; i++) {
      setCell(x,y,EMPTY);
      //setVariation(x,y,color(0,0,0));
      x += dx;
      y += dy;
    }
  }
}

void clearNumber(String number,int x,int y, int size) {
  for (int i = 0; i < number.length(); i++) {
    //clearDigit(number.charAt(i) - '0',x + (int)(i * size * 2),y,size);
  }
}

class Vector {
  public float x;
  public float y;
  public Vector(float nx, float ny) {
    x = nx;
    y = ny;
  }
  public Vector ofPolar(float direction, float magnitude) {
    x = magnitude * cos(direction);
    y = magnitude * sin(direction);
    return this;
  }
  public float direction() {
    return atan2(y,x);
  }
  public float magnitude() {
    return sqrt(x*x + y*y);
  }
   
   public Vector normalised() {
     return new Vector(
       x / magnitude(),
       y / magnitude()
     );
   }
   
   public Vector dif(Vector other) {
     return new Vector(
       other.x - x,
       other.y - y
     );
   }
   
   public Vector times(float amount) {
      return new Vector(
       x * amount,
       y * amount
     );
   }
  
  public void accelerateTowards(Vector pos,Vector point,float force) {
    Vector directionVector = pos.dif(point).normalised();
    
    x += directionVector.x * force / pos.dif(point).magnitude();
    y += directionVector.y * force / pos.dif(point).magnitude();
    
  }
  @Override
  public String toString() {
    return "[" + x + ", " + y + "]";
  }
}



void setup() {
  size(400,400);
  planetCore = new Vector(width/2,height/2);
  swaps = new boolean[width * height];
  velocities = new Vector[width * height];
  variation = new int[width * height];
  
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      setSwaps(x,y,false);
      setVelocity(x,y,new Vector(0,0));
    }
  }
  
  colourAngles.put(YELLOW,PI*2);
  colourWeights.put(YELLOW,2.0);
  colourAngles.put(BLUE,PI*2);
  colourWeights.put(BLUE,5.0);
  colourWeights.put(TEXT_COLOUR,10000.0);
  colourWeights.put(EMPTY,0.0);
  
  int defaultMat = EMPTY;
  
  background(defaultMat);
  //loadPixels();
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      //float decrease = 5;s
      //float increase = 5;
      
      //float rVal = red(defaultMat);
      //float gVal = green(defaultMat);
      //float bVal = blue(defaultMat);
      
      //float rDecrease = rVal - decrease < 0 ? rVal : decrease;
      //float gDecrease = gVal - decrease < 0 ? gVal : decrease;
      //float bDecrease = bVal - decrease < 0 ? bVal : decrease;
      
      //float rIncrease = rVal + increase > 256 ? 256 - rVal : increase;
      //float gIncrease = gVal + increase > 256 ? 256 - gVal : increase;
      //float bIncrease = bVal + increase > 256 ? 256 - bVal : increase;
      setVariation(x,y,color(
       32,32,32
     ));
    }
  }
  //setCircle(width/2,height/2,20,YELLOW);
  //updatePixels();
}

float lastTime = 0;
float delta = 0;
float totalTime = 0;

void draw() {
  delta = (millis() / 1000.0f) - lastTime;
  totalTime += delta;
      // Save current drawing settings
  
  updatePixels();
  loadPixels();
  //clearNumber("" + pixelCount,10,10,5);

  process(delta);

  
  
  // draw code here
  lastTime = (millis() / 1000.0f);
}

void spawn(int material,int x, int y,float angle,float magnitude) {
  setCell(x,y,material);
  setVelocity(x,y,new Vector(0,0).ofPolar(angle,magnitude));
  
  float decrease = 8;
  float increase = 8;
  
  float rVal = red(material);
  float gVal = green(material);
  float bVal = blue(material);
  
  float rDecrease = rVal - decrease <= 0 ? rVal : decrease;
  float gDecrease = gVal - decrease <= 0 ? gVal : decrease;
  float bDecrease = bVal - decrease <= 0 ? bVal : decrease;
  
  float rIncrease = rVal + increase >= 255 ? 255 - rVal : increase;
  float gIncrease = gVal + increase >= 255 ? 255 - gVal : increase;
  float bIncrease = bVal + increase >= 255 ? 255 - bVal : increase;

  setVariation(x,y,color(
    128 + rDecrease * random(rDecrease) - rIncrease * random(rIncrease),
    128 + gDecrease * random(gDecrease) - gIncrease * random(gIncrease),
    128 + bDecrease * random(bDecrease) - bIncrease * random(bIncrease)
   ));
}



void spawnCircle(int material,int x, int y, int radius,float angle,float magnitude) {
  for (int i = x - radius; i < x + radius; i++) {
     for (int j = y - radius; j < y + radius; j++) {
       if ((i-x)*(i-x) + (j-y)*(j-y) <= radius * radius) {
          spawn(material,i,j,angle,magnitude);
       }
    }
  }
}

void process(float delta) {
  if (pixelCount < 20000) {
    //setCircle(width/2,height/2,20,BLUE);
    //spawnCircle(YELLOW,width / 4, height / 3,  5,0,initSpeed);
    spawnCircle(BLUE,3 * width / 4, 3 * height / 4,20,0,initSpeed);
    spawnCircle(BLUE,1 * width / 4, 1 * height / 4,20,0,initSpeed);
    spawnCircle(YELLOW,3 * width / 4, 1 * height / 4,20,0,initSpeed);
    spawnCircle(YELLOW,1 * width / 4, 3 * height / 4,20,0,initSpeed);
  }
  
  
  pixelCount = 0;
  
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      
      processPixel(x,y,delta);
    }
  }
  drawNumber("" + pixelCount,10,10,5);
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      setSwaps(x,y,false);
      applyVariation(x,y);
    }
  }
  updatePixels();
  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      removeVariation(x,y);
    }
  }
}

void applyVariation(int x, int y) {
  int col = pixels[x + (width * y)];
  float rOffset = red(getVariation(x,y)) - 128;
  float gOffset = green(getVariation(x,y)) - 128;
  float bOffset = blue(getVariation(x,y)) - 128;
  
  pixels[x + (width * y)] = color(
    red(col) - rOffset,
    green(col) - gOffset,
    blue(col) - bOffset
  );
}

void removeVariation(int x, int y) {
  int col = pixels[x + (width * y)];
  float rOffset = red(getVariation(x,y)) - 128;
  float gOffset = green(getVariation(x,y)) - 128;
  float bOffset = blue(getVariation(x,y)) - 128;
  
  pixels[x + (width * y)] = color(
    red(col) + rOffset,
    green(col) + gOffset,
    blue(col) + bOffset
  );
}

// If we check n neighbours and they're all occupied, set velocity back to 0
boolean isTrapped(int x, int y) {
  //float randomisation = 0.2;
  //int attempts = 3;
  //
  //int[] npos = new int[2];
  //
  //Vector velocity = getVelocity(x,y);
  ////float angle = colourAngles.get(colour);
  //for (int tries = 0; tries < attempts; tries++) {
  //  npos = cellAtAngle(x,y,velocity.magnitude() * ((1-randomisation) +random(randomisation)),new Vector(x,y).dif(planetCore).direction() * ((1-randomisation)/2 +random(randomisation)));
  //  int otherColour = getCell(npos[0],npos[1]);
  //  if (colourWeights.get(otherColour) < weight) {
  //    return false;
  //  }
  //}
  //return true;
  int colour = getCell(x,y);
  
  float weight = colourWeights.get(colour);
  int neighbourCount = 0;
  int maxCount = 9;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      int otherColour = getCell(x + i,y + j);
      if (otherColour != TEXT_COLOUR && colourWeights.get(otherColour) >= weight) {
        neighbourCount++;
      }
    }
  }
  
  return neighbourCount >= maxCount;
}

/**
 *  Update the pixel to fall downwards
 */
void processPixel(int x,int y,float delta) {
  if (getCell(x,y) == EMPTY || getCell(x,y) == TEXT_COLOUR) { return; }
  pixelCount++;
  Vector velocity = getVelocity(x,y);
  //velocity.accelerateTowards(
  //  velocity,
  //  velocity.times(2),
  //  -drag * delta
  // );
   
  velocity.accelerateTowards(
    new Vector(x,y),
    planetCore,
    gravity * delta
   );
   
  if (isTrapped(x,y)) {
    velocity.accelerateTowards(
    velocity.normalised(),
    velocity.normalised().times(5),
    -1
   );
    if (velocity.magnitude() <= 1) { velocity = new Vector(x,y).dif(planetCore).normalised().times(max(velocity.magnitude(),0.01)); } //velocity.normalised().times(velocity.magnitude() * 0.2); }
  }
   
  setVelocity(x,y,velocity);
  
  int colour = getCell(x,y);
  
  // Move in current direction
  int[] npos = cellAtAngle(x,y,velocity.magnitude(),velocity.direction());
  if (swap(x,y,npos[0],npos[1],false)) { return; }
  
  float angle = colourAngles.get(colour);
  for (int tries = 0; tries < 3; tries++) {
    npos = cellAtAngle(x,y,velocity.magnitude(),velocity.direction() + random(angle * 2) - angle);
    if (swap(x,y,npos[0],npos[1],false)) { return; }
  }
}

/**
 * Swap x and y with nx ny iff x,y is heavier
 * @return false if we should try other swaps
 */
boolean swap(int x, int y, int nx, int ny,boolean isFinal) {
  if (abs(x - nx) > speedCap || abs(y - ny) > speedCap ) { return true; }
  if (getCell(x,y) == EMPTY || getCell(x,y) == TEXT_COLOUR) { return true; }
  //if (x <= 0 || nx <= 0 ||
  //    x >= width-1 || nx >= width-1 ||
  //    y <= 0 || ny <= 0 ||
  //    y >= height-1 || ny >= height-1 ||
   if (checkSwaps(x,y) || checkSwaps(nx,ny)) {
        return true;
   }
   
   int otherVal = getCell(nx,ny);
   Vector otherVel = getVelocity(nx,ny);
   
   if (otherVal == TEXT_COLOUR) { 
     setVelocity(x,y,new Vector(0,0)); 
   }
   
   int val = getCell(x,y);
   
   float weight = colourWeights.get(val);
   float otherWeight = colourWeights.get(otherVal);
   int otherVariation = getVariation(nx,ny);
   if (weight > otherWeight) { // == EMPTY && getCell(x,y) != EMPTY) {
     Vector vel = getVelocity(x,y);
     if (vel.x > speedCap) { vel.x = speedCap; } 
     if (vel.y > speedCap) { vel.y = speedCap; } 
     if (otherVel.x > speedCap) { otherVel.x = speedCap; } 
     if (otherVel.y > speedCap) { otherVel.y = speedCap; } 
     float divider = 2;
     if (weight == 0 || otherWeight == 0) {
       divider = 1;
     }
       setVelocity(x,y,vel.normalised().times(
         (vel.magnitude() + otherVel.magnitude()) / divider
        ));
       setVelocity(nx,ny,otherVel.normalised().times(
         (vel.magnitude() + otherVel.magnitude()) / divider
       ));
     //}
     setCell(nx,ny,getCell(x,y));
     setVelocity(nx,ny,getVelocity(x,y));
     setVariation(nx,ny,getVariation(x,y));
     setCell(x,y,otherVal);
     setVelocity(x,y,otherVel);
     setVariation(x,y,otherVariation);
     // Mark which cells have swapped
     setSwaps(x,y,true);
     setSwaps(nx,ny,true);
     return true;
   }  else if (!isFinal){
     return false;
   }  else {
     Vector vel = getVelocity(x,y);
     if (vel.x > speedCap) { vel.x = speedCap; } 
     if (vel.y > speedCap) { vel.y = speedCap; } 
     if (otherVel.x > speedCap) { otherVel.x = speedCap; } 
     if (otherVel.y > speedCap) { otherVel.y = speedCap; } 
     setVelocity(x,y,vel.normalised().times(
       (vel.magnitude() + otherVel.magnitude()) / 2.1
      ));
     setVelocity(nx,ny,otherVel.normalised().times(
       (vel.magnitude() + otherVel.magnitude()) / 2.1
     ));
     return false;
   }
}

// Return 2 element array of new position a float distance and angle from the current cell
int[] cellAtAngle(int x, int y, float distance, float angle) {
  
  // target position
  float tx = x + (distance * cos(angle));
  float ty = y + (distance * sin(angle));
  
  int nx = (int)tx;
  int ny = (int)ty;
  
  float resx = tx - nx;
  float resy = ty - ny;
  
  if (random(1) < resx) {
    nx++;
  }
  if (random(1) < resy) {
    ny++;
  }
  return new int[]{nx,ny};
}

int getCell(int x, int y) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return TEXT_COLOUR;
   }
   // Failsafe: just make sand if the pixel data is invalidated
  if (!colourWeights.containsKey(pixels[x + (y * width)])) { return YELLOW; }
  return pixels[x + (y * width)];
}

void setCell(int x, int y, int value) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return;
   }
  pixels[x + (y * width)] = value;
  //println(x + "," + y + " to " + value);
}

int getVariation(int x,int y) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return 0;
   }
  return variation[x + (y * width)];
}

void setVariation(int x,int y, int value) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return;
   }
  variation[x + (y * width)] = value;
  //println(x + "," + y + " to " + value);
}

void setCircle(int x, int y, int radius,int value) {
  for (int i = x - radius; i < x + radius; i++) {
     for (int j = y - radius; j < y + radius; j++) {
       if ((i-x)*(i-x) + (j-y)*(j-y) <= radius * radius) {
         setCell(i,j,value);
       }
    }
  }
}

void setVelocity(int x, int y, Vector value) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return;
   }
  velocities[x + (y * width)] = value;
}

Vector getVelocity(int x, int y) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return null;
   }
  return velocities[x + (y * width)];
}

boolean checkSwaps(int x, int y) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return true;
   }
  return swaps[x + (y * width)];
}

void setSwaps(int x, int y,boolean val) {
  if (x <= 0 || 
      x >= width-1 || 
      y <= 0 ||
      y >= height-1) {
        return;
   }
  swaps[x + (y * width)] = val;
}
