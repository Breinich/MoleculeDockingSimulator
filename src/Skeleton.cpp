//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Bajnok Vencel
// Neptun : X748Q2
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"

const char * const vertexSource = R"(
	#version 330
	precision highp float;

	uniform mat4 MVP;
	layout(location = 0) in vec2 vp;
    layout(location = 1) in vec3 vc;

    out vec3 color;

	void main() {
        color = vc;
		vec4 tmp = vec4(vp.x, vp.y, 0, 1) * MVP;
	    vec4 tmp1 = vec4(tmp.x, tmp.y, sqrt(tmp.x*tmp.x+tmp.y*tmp.y+1),1);
        gl_Position = vec4(tmp1.x/(tmp1.z + 1),tmp1.y/(tmp1.z + 1), 0,1 );
    }
)";

const char * const fragmentSource = R"(
	#version 330
	precision highp float;
	
	in vec3 color;
	out vec4 fragColor;

	void main() {
		fragColor = vec4(color, 1);
	}
)";

struct Camera{

    vec2 center;
    vec2 scale;

    Camera(){
        center = vec2(0,0);
        scale = vec2(2,2);
    }

    mat4 V() const {return TranslateMatrix(-center);}

    mat4 P() const { return ScaleMatrix(vec2( scale.x,  scale.y)); }

    void Pan(vec2 t) { center = center + t; }
};

GPUProgram gpuProgram;
Camera camera;
int circleVertexCount = 20;
int lineFragmentCount = 20;
const float PI = 3.1415926535;

struct Atom{

    int toltes;
    int tomeg;
    vec2 pos;
    Atom* szomszedok = nullptr;
    int szMeret;

    Atom(){
        toltes = rand()%10;
        toltes *= pow(-1, toltes);
        tomeg = rand()%5+1;
        pos.x = (float)((float)rand()/RAND_MAX* powf(-1, rand())*0.33f);
        pos.y = (float)((float)rand()/RAND_MAX* powf(-1, rand())*0.33f);
        szMeret = 0;
    }

    void AddSzomszed(Atom &a){
        Atom* uj = new Atom[++szMeret];
        for(int i = 0; i< szMeret-1; i++){
            uj[i] = szomszedok[i];
        }
        uj[szMeret-1] = a;
        if(szMeret-1 > 0)
            delete[] szomszedok;
        szomszedok = uj;
    }
};

struct Molekula{

    unsigned int vao;
    vec2 vel;
    vec2 pos;
    float tomeg;
    Atom* atomok;
    int count;
    int elekDuplan = 0;
    vec2 wTranslate;
    float omega;
    float phi;


    Molekula(){
        count = rand() % 7 + 2;
        atomok = new Atom[count];
        tomeg = 0;

        for(int i = 0; i < count; i++){

            bool vanEl = false;

            for(int j = 0; j < i-1; j++){

                int k = pow(-1, rand());
                if(k==1) {
                    atomok[j].AddSzomszed(atomok[i]);
                    elekDuplan+=2;
                    vanEl = true;
                    break;
                }
            }

            if(!vanEl && i>0){
                atomok[i-1].AddSzomszed(atomok[i]);
                elekDuplan+=2;
            }

            tomeg+= atomok[i].tomeg;
        }

        atomok[count - 1].toltes = 0;
        pos = vec2(0.0f,0.0f);

        for(int i = 0; i < count; i++){

            pos.x+= atomok[i].pos.x*(float)atomok[i].tomeg;
            pos.y+= atomok[i].pos.y*(float)atomok[i].tomeg;

            if(i!= count-1)
                atomok[count - 1].toltes += (-1) * atomok[i].toltes;
        }

        pos.x /= tomeg;
        pos.y /= tomeg;

        wTranslate.x = (float)rand()/RAND_MAX* powf(-1, rand())*0.5f;
        wTranslate.y = (float)rand()/RAND_MAX* powf(-1, rand())*0.5f;

        omega = 0;
        phi =0;
        vel = vec2(0,0);
    }

    void create(){

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        unsigned int vbo[2];
        glGenBuffers(2, &vbo[0]);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

        float vertexCoords[(count*(circleVertexCount+1)+elekDuplan*lineFragmentCount)*2];
        int index =0;

        for(int i = 0; i < count; i++){
            for(int j = 0; j < atomok[i].szMeret;j++) {

                vec2 delta = vec2((atomok[i].szomszedok[j].pos.x - atomok[i].pos.x)/(float)lineFragmentCount,
                                  (atomok[i].szomszedok[j].pos.y - atomok[i].pos.y)/(float)lineFragmentCount);

                for (int k = 0; k < lineFragmentCount; k++){
                    vertexCoords[index++] = atomok[i].pos.x + (float)k * delta.x;
                    vertexCoords[index++] = atomok[i].pos.y + (float)k * delta.y;
                    vertexCoords[index++] = atomok[i].pos.x + (float)(k+1) * delta.x;
                    vertexCoords[index++] = atomok[i].pos.y + (float)(k+1) * delta.y;
                }
            }
        }

        for(int i = 0; i < count; i++){

            vertexCoords[index++] = atomok[i].pos.x;
            vertexCoords[index++] = atomok[i].pos.y;

            for(int j = 0; j < circleVertexCount;j++){
                vertexCoords[index++] = (float)(atomok[i].pos.x + cosf((float)(j/(circleVertexCount-1.0)*2*PI))*(0.01+(float)atomok[i].tomeg/200));
                vertexCoords[index++] = (float)(atomok[i].pos.y + sinf((float)(j/(circleVertexCount-1.0)*2*PI))*(0.01+(float)atomok[i].tomeg/200));
            }
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexCoords), vertexCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE,	0, NULL);

        glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);

        float vertexColors[(count*(circleVertexCount+1)+elekDuplan*lineFragmentCount)*3];
        index = 0;

        for(int i = 0; i < elekDuplan*lineFragmentCount*3; i++){
            vertexColors[index++] = 1;
        }

        for(int i = 0; i < count; i++){

            if(atomok[i].toltes < 0){

                for(int j = 0; j < circleVertexCount+1;j++) {
                    vertexColors[index++] = 0;
                    vertexColors[index++] = 0;
                    vertexColors[index++] = (float)atomok[i].toltes / -15.0f;
                }
            }
            else{

                for(int j = 0; j < circleVertexCount+1;j++) {
                    vertexColors[index++] = (float) atomok[i].toltes / 15.0f;
                    vertexColors[index++] = 0;
                    vertexColors[index++] = 0;
                }
            }
        }

        glBufferData(GL_ARRAY_BUFFER, sizeof(vertexColors), vertexColors, GL_STATIC_DRAW);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    }

    mat4 M() const {

        mat4 MTrToRot(1,0,0,0,
                      0,1,0,0,
                      0,0,0,0,
                      -pos.x, -pos.y,0,1);

        mat4 Mrotate(cosf(phi), sinf(phi), 0, 0,
                     -sinf(phi), cosf(phi), 0, 0,
                     0,        0,        1, 0,
                     0,        0,        0, 1);

        mat4 MTrToRotBack(1,0,0,0,
                          0,1,0,0,
                          0,0,0,0,
                          pos.x, pos.y,0,1);

        mat4 Mtranslate(1,            0,            0, 0,
                        0,            1,            0, 0,
                        0,            0,            0, 0,
                        wTranslate.x, wTranslate.y, 0, 1);

        return MTrToRot* Mrotate *MTrToRotBack* Mtranslate;
    }

    void Draw() const {

        mat4 MVPTransform = M()*camera.V()*camera.P();
        gpuProgram.setUniform(MVPTransform, "MVP");

        glBindVertexArray(vao);

        for(int i = 0; i < elekDuplan*lineFragmentCount/2;i++)
            glDrawArrays(GL_LINE_STRIP, 0+2*i, 2);

        for(int i = 0; i<count;i++)
            glDrawArrays(GL_TRIANGLE_FAN, elekDuplan*lineFragmentCount+i*(circleVertexCount+1), circleVertexCount+1);
    }

    ~Molekula(){
        glDeleteVertexArrays(1, &vao);
    }
};

Molekula molekula1;
Molekula molekula2;

void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

    molekula1.create();
    molekula2.create();


	gpuProgram.create(vertexSource, fragmentSource, "fragColor");
}


void onDisplay() {
	glClearColor(0.3, 0.3, 0.3, 0);
	glClear(GL_COLOR_BUFFER_BIT);

    molekula1.Draw();
    molekula2.Draw();

	glutSwapBuffers();
}

void onKeyboard(unsigned char key, int pX, int pY) {
    switch (key) {
        case 's':
            camera.Pan(vec2(-0.1, 0));
            break;
        case 'd':
            camera.Pan(vec2(+0.1, 0));
            break;
        case 'e':
            camera.Pan(vec2(0, 0.1));
            break;
        case 'x':
            camera.Pan(vec2(0, -0.1));
            break;
        case 32:
            molekula1 = Molekula();
            molekula1.create();
            molekula2 = Molekula();
            molekula2.create();

            break;

        default:
            break;
    }
    glutPostRedisplay();
}

void onKeyboardUp(unsigned char key, int pX, int pY) {}

void onMouseMotion(int pX, int pY) {}

void onMouse(int button, int state, int pX, int pY) {}

float distance(vec4 p1, vec4 p2){
    float d = sqrtf(powf(p1.x-p2.y, 2)+powf(p1.y -p2.y,2));
    return d;
}

long lastTime = 0;

void onIdle() {

	long time = glutGet(GLUT_ELAPSED_TIME);

    long dtmilli = 1;
    const float hidrogen = 1.0079f / 6.02214f;
    const float epsilon = 8.8541878f;
    const float electron = 1.602f;
    const int hidrOrdo = -23;
    const int elecOrdo =  -19;
    const int distanceOrdo = -2;
    const int epsOrdo = -12;
    const float stokes = 300.0f;
    float sumOrdo = powf(10, elecOrdo*2-epsOrdo-distanceOrdo-hidrOrdo);

    for(long t = lastTime; t < time; t+= dtmilli){

        float forgato1 = 0.0f;
        float teta1 = 0.0f;
        vec2 eroSum1 = vec2(0, 0);
        vec2 coulombsum1 = vec2(0,0);

        for(int i = 0; i < molekula1.count; i++){

            vec2 localCoulomb = vec2(0,0);
            vec4 atom1pos = vec4(molekula1.atomok[i].pos.x, molekula1.atomok[i].pos.y, 0, 0)*molekula1.M();
            vec4 molekula1pos = vec4(molekula1.pos.x, molekula1.pos.y, 0, 0)*molekula1.M();

            for(int j = 0; j < molekula2.count; j++){
                vec4 atom2pos = vec4(molekula2.atomok[j].pos.x,molekula2.atomok[j].pos.y,0,0)*molekula2.M();

                localCoulomb = localCoulomb + (float)(molekula1.atomok[i].toltes*molekula2.atomok[j].toltes)*electron*electron/
                                              (2*PI*distance(atom1pos, atom2pos)*epsilon)*
                                              normalize(vec2(atom1pos.x, atom1pos.y)-vec2(atom2pos.x, atom2pos.y));
            }

            float keruletiero = (atom1pos.x-molekula1pos.x)*localCoulomb.y - (atom1pos.y-molekula1pos.y)*localCoulomb.x;
            forgato1 = forgato1 + keruletiero;

            teta1 += powf(length(vec2(atom1pos.x-molekula1pos.x, atom1pos.y-molekula1pos.y)),2)*(float)molekula1.atomok[i].tomeg;
            coulombsum1 = coulombsum1 + localCoulomb;
        }

        eroSum1 = eroSum1 + coulombsum1;
        eroSum1 = eroSum1 - stokes*molekula1.vel;

        molekula1.vel = molekula1.vel + eroSum1 / (molekula1.tomeg*hidrogen)*(float)dtmilli/1000.0f*sumOrdo;
        molekula1.wTranslate = molekula1.wTranslate + molekula1.vel*(float)dtmilli/1000.0f;

        forgato1 -= sqrtf(stokes)*molekula1.omega;
        molekula1.omega += forgato1 / teta1 *sumOrdo*(float)dtmilli/1000.0f;
        molekula1.phi += molekula1.omega* (float)dtmilli/1000.0f;


        float forgato2 = 0.0f;
        float teta2 = 0.0f;
        vec2 eroSum2 = vec2(0, 0);
        vec2 coulombsum2 = vec2(0,0);

        for(int i = 0; i < molekula2.count; i++){

            vec2 localCoulomb = vec2(0,0);
            vec4 atom2pos = vec4(molekula2.atomok[i].pos.x, molekula2.atomok[i].pos.y, 0, 0)*molekula2.M();
            vec4 molekula2pos = vec4(molekula2.pos.x, molekula2.pos.y, 0, 0)*molekula2.M();

            for(int j = 0; j < molekula1.count; j++){
                vec4 atom1pos = vec4(molekula1.atomok[j].pos.x,molekula1.atomok[j].pos.y,0,0)*molekula2.M();

                localCoulomb = localCoulomb + (float)(molekula2.atomok[i].toltes*molekula1.atomok[j].toltes)*electron*electron/
                                              (2*PI*distance(atom2pos, atom1pos)*epsilon)*
                                              normalize(vec2(atom2pos.x, atom2pos.y)-vec2(atom1pos.x, atom1pos.y));
            }

            float keruletiero = (atom2pos.x-molekula2pos.x)*localCoulomb.y - (atom2pos.y-molekula2pos.y)*localCoulomb.x;
            forgato2 = forgato2 + keruletiero;

            teta2 += powf(length(vec2(atom2pos.x-molekula2pos.x,atom2pos.y-molekula2pos.y)),2)*(float)molekula2.atomok[i].tomeg;
            coulombsum2 = coulombsum2 + localCoulomb;
        }

        eroSum2 = eroSum2 + coulombsum2;
        eroSum2 = eroSum2 + -stokes*molekula2.vel;

        molekula2.vel = molekula2.vel + eroSum2 / (molekula2.tomeg*hidrogen)*(float)dtmilli/1000.0f*sumOrdo;
        molekula2.wTranslate = molekula2.wTranslate + molekula2.vel*(float)dtmilli/1000.0f;

        forgato2 -= sqrtf(stokes)*molekula2.omega;
        molekula2.omega += forgato2 / teta2 *sumOrdo*(float)dtmilli/1000.0f;
        molekula2.phi += molekula2.omega* (float)dtmilli/1000.0f;
    }
    lastTime = time;

    glutPostRedisplay();
}
