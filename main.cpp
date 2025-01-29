//g++ main.cpp -framework OpenGL -framework GLUT -Wno-deprecated
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <GLUT/glut.h>  //OpenGL/GLUTの使用

//定数宣言
#define FLOOR 500.0  //床の幅の半分
#define TILE 50  //床頂点格子分割数
#define TEX_SIZE 512  //テクスチャサイズ

//三次元ベクトル構造体
typedef struct _Vec_3D
{
    double x, y, z;
} Vec_3D;

//関数名の宣言
void initGL();
void display();
void reshape(int w, int h);
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void normcrossprod(Vec_3D v1, Vec_3D v2, Vec_3D* out);  //外積計算
double vectorNormalize(Vec_3D* vec);   //ベクトル正規化
void scene();

//グローバル変数
double eDist, eDegX, eDegY;  //視点極座標
int mX, mY, mState, mButton;  //マウス座標
int winW, winH;  //ウィンドウサイズ
Vec_3D floorPoint[TILE][TILE];  //床頂点
Vec_3D pPoint[6][4];  //箱頂点
double bottom = 0.0;
double fr = 30.0;
//テクスチャ生成関数パラメータ
static double genfunc[][4] = {
    {1.0, 0.0, 0.0, 0.0},
    {0.0, 1.0, 0.0, 0.0},
    {0.0, 0.0, 1.0, 0.0},
    {0.0, 0.0, 0.0, 1.0}
};
double theta = 0.0;

//メイン関数
int main(int argc, char *argv[])
{
    glutInit(&argc, argv);  //OpenGL/GLUTの初期化
    initGL();  //初期設定
    
    glutMainLoop();  //イベント待ち無限ループ
    
    return 0;
}

//初期化関数
void initGL()
{
    //描画ウィンドウ生成
    glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);  //ディスプレイモードの指定
    glutInitWindowSize(800, 600);  //ウィンドウサイズの指定
    glutCreateWindow("CGT");  //ウィンドウの生成
    
    //コールバック関数の指定
    glutDisplayFunc(display);  //ディスプレイコールバック関数の指定
    glutReshapeFunc(reshape);  //リシェイプコールバック関数の指定
    glutMouseFunc(mouse);  //マウスクリックコールバック関数の指定
    glutMotionFunc(motion);  //マウスドラッグコールバック関数の指定
    glutKeyboardFunc(keyboard);  //キーボードコールバック関数の指定
    glutTimerFunc(1000/fr, timer, 0);  //タイマー
    
    //各種設定
    glClearColor(0.0, 0.3, 0.5, 1.0);  //ウィンドウクリア色の指定（RGBA）
    glEnable(GL_DEPTH_TEST);  //デプスバッファの有効化
    glEnable(GL_NORMALIZE);  //ベクトル正規化有効化
    glEnable(GL_BLEND);  //アルファブレンド有効化
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  //ブレンド方法の設定
    glEnable(GL_ALPHA_TEST);  //アルファテスト有効化
    glAlphaFunc(GL_GREATER, 0.01);  //アルファ値比較関数の設定
    
    //光源設定
    GLfloat col[4] = {0.8, 0.8, 0.8, 1.0};  //パラメータ(RGBA)
    glEnable(GL_LIGHTING);  //陰影付けの有効化
    glEnable(GL_LIGHT0);  //光源0の有効化
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源1の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源1の鏡面反射の強度
    col[0] = 0.2; col[1] = 0.2; col[2] = 0.2; col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_AMBIENT, col);  //光源1の環境光の強度
    
    //視点関係
    eDist = 1500.0; eDegX = 10.0; eDegY = 0.0;  //視点極座標
    
    //床頂点座標
    for (int i=0; i<TILE; i++) {
        for (int j=0; j<TILE; j++) {
            floorPoint[i][j].x = -FLOOR+2*FLOOR/(TILE-1)*i;  //x座標
            floorPoint[i][j].y = bottom;  //y座標(高さ)
            floorPoint[i][j].z = -FLOOR+2*FLOOR/(TILE-1)*j;  //z座標
        }
    }
    
    //箱
    pPoint[0][0].x = -1.0; pPoint[0][0].y = 1.0; pPoint[0][0].z = -1.0;
    pPoint[0][1].x = 1.0; pPoint[0][1].y = 1.0; pPoint[0][1].z = -1.0;
    pPoint[0][2].x = 1.0; pPoint[0][2].y = 1.0; pPoint[0][2].z = 1.0;
    pPoint[0][3].x = -1.0; pPoint[0][3].y = 1.0; pPoint[0][3].z = 1.0;
    
    pPoint[1][0].x = 1.0; pPoint[1][0].y = -1.0; pPoint[1][0].z = -1.0;
    pPoint[1][1].x = -1.0; pPoint[1][1].y = -1.0; pPoint[1][1].z = -1.0;
    pPoint[1][2].x = -1.0; pPoint[1][2].y = -1.0; pPoint[1][2].z = 1.0;
    pPoint[1][3].x = 1.0; pPoint[1][3].y = -1.0; pPoint[1][3].z = 1.0;
    
    pPoint[2][0].x = -1.0; pPoint[2][0].y = 1.0; pPoint[2][0].z = 1.0;
    pPoint[2][1].x = 1.0; pPoint[2][1].y = 1.0; pPoint[2][1].z = 1.0;
    pPoint[2][2].x = 1.0; pPoint[2][2].y = -1.0; pPoint[2][2].z = 1.0;
    pPoint[2][3].x = -1.0; pPoint[2][3].y = -1.0; pPoint[2][3].z = 1.0;
    
    pPoint[3][0].x = 1.0; pPoint[3][0].y = 1.0; pPoint[3][0].z = -1.0;
    pPoint[3][1].x = -1.0; pPoint[3][1].y = 1.0; pPoint[3][1].z = -1.0;
    pPoint[3][2].x = -1.0; pPoint[3][2].y = -1.0; pPoint[3][2].z = -1.0;
    pPoint[3][3].x = 1.0; pPoint[3][3].y = -1.0; pPoint[3][3].z = -1.0;
    
    pPoint[4][0].x = 1.0; pPoint[4][0].y = 1.0; pPoint[4][0].z = 1.0;
    pPoint[4][1].x = 1.0; pPoint[4][1].y = 1.0; pPoint[4][1].z = -1.0;
    pPoint[4][2].x = 1.0; pPoint[4][2].y = -1.0; pPoint[4][2].z = -1.0;
    pPoint[4][3].x = 1.0; pPoint[4][3].y = -1.0; pPoint[4][3].z = 1.0;
    
    pPoint[5][0].x = -1.0; pPoint[5][0].y = 1.0; pPoint[5][0].z = -1.0;
    pPoint[5][1].x = -1.0; pPoint[5][1].y = 1.0; pPoint[5][1].z = 1.0;
    pPoint[5][2].x = -1.0; pPoint[5][2].y = -1.0; pPoint[5][2].z = 1.0;
    pPoint[5][3].x = -1.0; pPoint[5][3].y = -1.0; pPoint[5][3].z = -1.0;
    
    //テクスチャ生成
    glBindTexture(GL_TEXTURE_2D, 0);  //テクスチャ#0
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);  //s方向繰り返し設定(GL_CLAMP/GL_REPEAT)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);  //t方向繰り返し設定(GL_CLAMP/GL_REPEAT)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //テクスチャへの画像適用
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEX_SIZE, TEX_SIZE, 0, GL_BGRA, GL_UNSIGNED_BYTE, image0);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, TEX_SIZE, TEX_SIZE, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, TEX_SIZE, TEX_SIZE, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_ALPHA);
    //テクスチャ合成方法
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    
//    //頂点のオブジェクト空間座標値をテクスチャ座標として使用
//    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
//    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
//    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
//    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_OBJECT_LINEAR);
    //頂点のワールド空間座標値をテクスチャ座標として使用
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
    glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);

//    //テクスチャ座標生成関数の設定
//    glTexGendv(GL_S, GL_OBJECT_PLANE, genfunc[0]);
//    glTexGendv(GL_T, GL_OBJECT_PLANE, genfunc[1]);
//    glTexGendv(GL_R, GL_OBJECT_PLANE, genfunc[2]);
//    glTexGendv(GL_Q, GL_OBJECT_PLANE, genfunc[3]);
}

//ディスプレイコールバック関数
void display()
{

    //視点座標の計算
    Vec_3D e;
    e.x = eDist*cos(eDegX*M_PI/180.0)*sin(eDegY*M_PI/180.0);
    e.y = eDist*sin(eDegX*M_PI/180.0);
    e.z = eDist*cos(eDegX*M_PI/180.0)*cos(eDegY*M_PI/180.0);
    
    //光源0の配置
    // GLfloat lightPos0[] = {800.0, 600.0, 400.0, 1.0};  //点光源
    GLfloat lightPos0[4];  //点光源
    lightPos0[0] = 1200.0*sin(theta);
    lightPos0[1] = 600.0;
    lightPos0[2] = 1200.0*cos(theta);
    lightPos0[3] = 1.0;
    theta += 0.01;

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos0);  //光源座標
    
    //光源0から見た画像の生成
    GLint viewport[4];
    GLdouble projection[16];
    
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //画面消去
    
    //現状保存
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);

    glViewport(0, 0, TEX_SIZE, TEX_SIZE);
    glMatrixMode(GL_PROJECTION);
    
    // 光源座標から見たCG映像の生成
    glLoadIdentity();
    gluPerspective(40.0, (double)TEX_SIZE/(double)TEX_SIZE, 100.0, 2500.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(lightPos0[0], lightPos0[1], lightPos0[2], 0, 0, 0, 0, 1, 0);
    scene();
    glCopyTexSubImage2D(GL_TEXTURE_2D, 0,0,0,0,0, TEX_SIZE, TEX_SIZE);
    
    //元に戻す
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(projection);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  //画面消去

    //テクスチャ座標変換
    glMatrixMode(GL_TEXTURE);  //変換行列の指定（テクスチャ行列）
    glLoadIdentity();  //行列初期化
    glTranslated(0.5, 0.5, 0.5);
    glScaled(0.5, 0.5, 0.5);
    gluPerspective(40.0, (double)TEX_SIZE/(double)TEX_SIZE, 101.0, 2500.0);
    gluLookAt(lightPos0[0], lightPos0[1], lightPos0[2], 0, 0, 0, 0, 1, 0);
    
    //モデルビュー変換の設定
    glMatrixMode(GL_MODELVIEW);  //変換行列の指定（設定対象はモデルビュー変換行列）
    glLoadIdentity();  //行列初期化
    gluLookAt(e.x, e.y, e.z, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);  //視点視線設定（視野変換行列を乗算）
    
    //光源ありCG生成用
    GLfloat col[4];  //パラメータ(RGBA)
    col[0] = 0.8;
    col[1] = 0.8;
    col[2] = 0.8;
    col[3] = 1.0;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源0の鏡面反射の強度

    //テクスチャ座標生成関数の設定
    glTexGendv(GL_S, GL_EYE_PLANE, genfunc[0]);
    glTexGendv(GL_T, GL_EYE_PLANE, genfunc[1]);
    glTexGendv(GL_R, GL_EYE_PLANE, genfunc[2]);
    glTexGendv(GL_Q, GL_EYE_PLANE, genfunc[3]);

    //テクスチャ有効化
    glEnable(GL_TEXTURE_2D);
    //テクスチャ座標自動生成有効化
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
    glEnable(GL_TEXTURE_GEN_Q);

    //シーン描画
    scene();
    
    //テクスチャ座標自動生成無効化
    glDisable(GL_TEXTURE_GEN_S);
    glDisable(GL_TEXTURE_GEN_T);
    glDisable(GL_TEXTURE_GEN_R);
    glDisable(GL_TEXTURE_GEN_Q);

    //テクスチャ無効化
    glDisable(GL_TEXTURE_2D);

    // 光源なしCG生成用
    col[0] = 0.0;
    col[1] = 0.0;
    col[2] = 0.0;
    col[3] = 0.8;
    glLightfv(GL_LIGHT0, GL_DIFFUSE, col);  //光源0の拡散反射の強度
    glLightfv(GL_LIGHT0, GL_SPECULAR, col);  //光源0の鏡面反射の強度
    scene();
    
    glutSwapBuffers();  //描画実行
}

void scene()
{
    //描画準備
    GLfloat col[4], spe[4], shi[1];  //材質設定用配列
    Vec_3D v1, v2, nv;  //辺ベクトル，法線ベクトル
    
    //床面
    //色の設定
    col[0] = 0.8; col[1] = 0.8; col[2] = 0.2; col[3] = 1.0;  //拡散反射係数
    spe[0] = 1.0; spe[1] = 1.0; spe[2] = 1.0; spe[3] = 1.0;  //鏡面反射係数
    shi[0] = 100.0;  //ハイライト係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, spe);  //鏡面反射
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shi);  //ハイライト
    //頂点配置
    glBegin(GL_QUADS);
    for (int j=0; j<TILE-1; j++) {
        for (int i=0; i<TILE-1; i++) {
            //法線ベクトル計算
            for (int k=0; k<3; k++) {
                //辺ベクトル1
                v1.x = floorPoint[i][j+1].x-floorPoint[i][j].x;
                v1.y = floorPoint[i][j+1].y-floorPoint[i][j].y;
                v1.z = floorPoint[i][j+1].z-floorPoint[i][j].z;
                //辺ベクトル2
                v2.x = floorPoint[i+1][j].x-floorPoint[i][j].x;
                v2.y = floorPoint[i+1][j].y-floorPoint[i][j].y;
                v2.z = floorPoint[i+1][j].z-floorPoint[i][j].z;
            }
            normcrossprod(v1, v2, &nv);  //辺ベクトル外積計算→法線ベクトル
            glNormal3d(nv.x, nv.y, nv.z);  //法線ベクトル設定
            //頂点座標
            glVertex3d(floorPoint[i][j].x, floorPoint[i][j].y, floorPoint[i][j].z);  //座標
            glVertex3d(floorPoint[i][j+1].x, floorPoint[i][j+1].y, floorPoint[i][j+1].z);  //座標
            glVertex3d(floorPoint[i+1][j+1].x, floorPoint[i+1][j+1].y, floorPoint[i+1][j+1].z);  //座標
            glVertex3d(floorPoint[i+1][j].x, floorPoint[i+1][j].y, floorPoint[i+1][j].z);  //座標
        }
    }
    glEnd();  //配置終了
    
    glEnable(GL_ALPHA_TEST);  //アルファテスト有効化
    
    //箱の描画
    //色
    col[0] = 0.5; col[1] = 0.5; col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光係数
    glPushMatrix();  //行列一時保存
    glTranslated(0.0, 110.0, 0.0);  //平行移動
    glScaled(100.0, 100.0, 100.0);  //拡大
    //6面
    for (int i=0; i<6; i++) {
        glBegin(GL_QUADS);
        //法線ベクトル計算
        //辺v1
        v1.x = pPoint[i][3].x-pPoint[i][0].x;
        v1.y = pPoint[i][3].y-pPoint[i][0].y;
        v1.z = pPoint[i][3].z-pPoint[i][0].z;
        //辺v2
        v2.x = pPoint[i][1].x-pPoint[i][0].x;
        v2.y = pPoint[i][1].y-pPoint[i][0].y;
        v2.z = pPoint[i][1].z-pPoint[i][0].z;
        normcrossprod(v1, v2, &nv);  //v1とv2の外積nv
        glNormal3d(nv.x, nv.y, nv.z);  //法線ベクトル設定
        
        glVertex3d(pPoint[i][0].x, pPoint[i][0].y, pPoint[i][0].z);  //頂点座標
        glVertex3d(pPoint[i][1].x, pPoint[i][1].y, pPoint[i][1].z);  //頂点座標
        glVertex3d(pPoint[i][2].x, pPoint[i][2].y, pPoint[i][2].z);  //頂点座標
        glVertex3d(pPoint[i][3].x, pPoint[i][3].y, pPoint[i][3].z);  //頂点座標
        
        glEnd();  //配置終了
    }
    glPopMatrix();  //一時保存行列復帰
    
    //球の描画
    //色
    col[0] = 1.0; col[1] = 1.0; col[2] = 1.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光係数
    glPushMatrix();  //行列一時保存
    glTranslated(0.0, 300.0, 0.0);  //平行移動
    glScaled(70.0, 70.0, 70.0);  //拡大
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();  //一時保存行列復帰

    //球の描画
    //色
    col[0] = 1.0; col[1] = 1.0; col[2] = 0.0;
    glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, col);  //拡散反射係数
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, col);  //環境光係数
    glPushMatrix();  //行列一時保存
    glTranslated(300.0*sin(theta*2), 40.0, 0.0);  //平行移動
    glScaled(40.0, 40.0, 40.0);  //拡大
    glutSolidSphere(1.0, 36, 18);
    glPopMatrix();  //一時保存行列復帰
    
    //glDisable(GL_ALPHA_TEST); //アルファテスト無効化
}

//リシェイプコールバック関数
void reshape(int w, int h)
{
    glViewport(0, 0, w, h);  //ビューポート設定
    //投影変換の設定
    glMatrixMode(GL_PROJECTION);  //変換行列の指定（設定対象は投影変換行列）
    glLoadIdentity();  //行列初期化
    gluPerspective(30.0, (double)w/(double)h, 100.0, 5000.0);  //透視投影ビューボリューム設定
    
    winW = w; winH = h;  //ウィンドウサイズ格納
}

//マウスクリックコールバック関数
void mouse(int button, int state, int x, int y)
{
    GLdouble model[16], proj[16];  //変換行列格納用
    GLint view[4];  //ビューポート設定格納用
    GLfloat win_x, win_y, win_z;  //ウィンドウ座標
    GLdouble obj_x, obj_y, obj_z;  //ワールド座標
    
    //マウスボタンが押された
    if (state==GLUT_DOWN) {
        mX = x; mY = y; mState = state; mButton = button;  //マウス情報保持
        //左ボタンのとき
        if (button==GLUT_LEFT_BUTTON) {
            //マウス座標からウィンドウ座標を取得
            win_x = mX; win_y =winH-mY;  //x座標，y座標
            glReadPixels(win_x, win_y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &win_z);  //z座標
            
            //変換行列，ビューポート設定取り出し
            glGetDoublev(GL_MODELVIEW_MATRIX, model);  //モデルビュー変換行列
            glGetDoublev(GL_PROJECTION_MATRIX, proj);  //投影変換行列
            glGetIntegerv(GL_VIEWPORT, view);  //ビューポート設定
            
            //ウィンドウ座標をワールド座標に変換
            gluUnProject(win_x, win_y, win_z, model, proj, view, &obj_x, &obj_y, &obj_z);
            
            //表示（確認用）
            printf("mx = %d, my = %d\n", mX, mY);
            printf("win_x = %f, win_y = %f, win_z = %f\n", win_x, win_y, win_z);
            printf("obj_x = %f, obj_y = %f, obj_z = %f\n\n", obj_x, obj_y, obj_z);
        }
    }
}

//マウスドラッグコールバック関数
void motion(int x, int y)
{
    if (mButton==GLUT_RIGHT_BUTTON) {
        //マウスの移動量を角度変化量に変換
        eDegY = eDegY+(mX-x)*0.5;  //マウス横方向→水平角
        eDegX = eDegX+(y-mY)*0.5;  //マウス縦方向→垂直角
    }
    
    //マウス座標をグローバル変数に保存
    mX = x; mY = y;
}

//キーボードコールバック関数(key:キーの種類，x,y:座標)
void keyboard(unsigned char key, int x, int y)
{
    switch (key) {
        case 'q':
        case 'Q':
        case '\033':  // '\033'はESCのASCIIコード
            exit(0);
    }
}

//タイマーコールバック関数
void timer(int value)
{
    glutPostRedisplay();  //ディスプレイイベント強制発生
    glutTimerFunc(1000/fr, timer, 0);  //タイマー再設定
}

//ベクトルの外積計算
void normcrossprod(Vec_3D v1, Vec_3D v2, Vec_3D* out)
{
    //外積の定義に基づき各成分計算
    out->x = v1.y*v2.z-v1.z*v2.y;
    out->y = v1.z*v2.x-v1.x*v2.z;
    out->z = v1.x*v2.y-v1.y*v2.x;
    
    //正規化
    vectorNormalize(out);
}

//ベクトル正規化
double vectorNormalize(Vec_3D* vec)
{
    //ベクトル長
    double length = sqrt(vec->x*vec->x+vec->y*vec->y+vec->z*vec->z);
    
    //正規化
    if (length>0) {
        vec->x = vec->x/length;
        vec->y = vec->y/length;
        vec->z = vec->z/length;
    }
    else {
        vec->x = 0.0;
        vec->y = 0.0;
        vec->z = 0.0;
    }
    
    //戻り値はベクトル長
    return length;
}
