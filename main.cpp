#include<iostream>
#include<graphics.h>
#include<cmath>
#include<cstring>
#include<algorithm>
#include<mmsystem.h>
#include<vector>
#include<stack>
#include<queue>
#include<fstream>
#pragma comment(lib,"winmm.lib")


using namespace std;
void BGM();
void ShowHelp();
void JudgeRes();
void OnePlay();
void TwoPlay();
void Play();	//���Ծ�������ĺ���������Ҫ�㷨����
void Save();
void Load();
void Save2();
void Load2();
inline pair<int, int> Choose_cross(int x, int y);



const int WID = 1000;
const int HEI = 740;
bool mutex = true;			//Ϊ0ʱ�������ã�Ϊ1ʱ�ǿ���״̬
bool play_music = false;
int is_over = 0;		//0��ʾδ������1��ʾ��ʤ��-1��ʾ��ʤ
int player_color = 0;	//��ҵ���ɫ��0��ʾ�ڷ���1��ʾ�׷�
int curcolor = 0;		//˫����Ϸ�б��ֵ���ɫ
const int cross_posx[9] = { 307,387,467,549,629,710,791,871,951};
const int cross_posy[9] = { 46,127,207,288,368,449,530,609,690 };
int Board[9][9] = { 0 };	//1 black -1 white 0 empty
int actionR[2];				//��¼������һ����
int AIactionR[2];			//�˻���ս�м�¼AI����һ����



const int MCTSn = 64;
const double C = 1.0 / sqrt(2);

int dxdy[4][2] = { {1,0},{0,-1},{-1,0},{0,1} };
int exey[4][2] = { {1,1},{1,-1},{-1,1},{-1,-1} };


bool dfs_air_visit[9][9];
const int cx[] = { -1,0,1,0 };
const int cy[] = { 0,-1,0,1 };
int Color;

bool inBorder(int x, int y)
{
	return x >= 0 && y >= 0 && x < 9 && y < 9;
}

//true: has air
bool dfs_air(int fx, int fy)
{
	dfs_air_visit[fx][fy] = true;
	bool flag = false;
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (Board[dx][dy] == 0)
				flag = true;
			if (Board[dx][dy] == Board[fx][fy] && !dfs_air_visit[dx][dy])
				if (dfs_air(dx, dy))
					flag = true;
		}
	}
	return flag;
}

//true: available
bool judgeAvailable(int fx, int fy, int col)
{
	Board[fx][fy] = col;
	memset(dfs_air_visit, 0, sizeof(dfs_air_visit));
	if (!dfs_air(fx, fy))
	{
		Board[fx][fy] = 0;
		return false;
	}
	for (int dir = 0; dir < 4; dir++)
	{
		int dx = fx + cx[dir], dy = fy + cy[dir];
		if (inBorder(dx, dy))
		{
			if (Board[dx][dy] && !dfs_air_visit[dx][dy])
				if (!dfs_air(dx, dy))
				{
					Board[fx][fy] = 0;
					return false;
				}
		}
	}
	Board[fx][fy] = 0;
	return true;
}


//true: game end
int CountBu1(int col)
{
	for (int i = 0; i < 9; i++)
		for (int j = 0; j < 9; j++)
			if (Board[i][j] == 0 && judgeAvailable(i, j, col))
				return false;
	return true;
}

bool ProcStep(int x, int y, int color, bool checkOnly = false)
{
	if (!inBorder(x, y) || Board[x][y])
		return false;
	if (!judgeAvailable(x, y, color))
		return false;
	if (!checkOnly)
		Board[x][y] = color;
	return true;
}

vector<int> best_choice;

class node
{
public:
	int id;
	int score;
	node() {}
	node(int x, int y, int sco)
	{
		id = x * 9 + y;
		score = sco;
	}
	bool operator < (const node& A) const
	{
		return score > A.score;
	}
};

/*
�ж��Ƿ���˫����������ĵ㣬���û�о�תΪ���������ѡһ���Ϸ���
*/
bool createmove()
{
	int a1, a2, b1, b2, a3, b3;
	int da1, da2, db1, db2;
	best_choice.clear();
	vector<node> v;
	bool flag = false;
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (judgeAvailable(i, j, Color) && judgeAvailable(i, j, -Color))
			{
				flag = true;
				b1 = CountBu1(Color);
				a1 = CountBu1(-Color);
				Board[i][j] = Color;
				b2 = CountBu1(Color);
				a2 = CountBu1(-Color);
				Board[i][j] = -Color;
				b3 = CountBu1(Color);
				a3 = CountBu1(-Color);
				Board[i][j] = 0;
				da1 = a1 - a2, da2 = a1 - a3, db1 = b1 - b2, db2 = b1 - b3;
				int sco = da1 - db1 - da2 + db2;
				v.push_back(node(i, j, sco));
			}
		}
	}
	if (!flag)	//û��˫���������µĵ��ˣ���ʱ��תΪ�������
	{
		return false;
	}
	sort(v.begin(), v.end());
	int maxstep = -200;
	int len = v.size();
	for (int i = 0; i < len; i++)
	{
		if (v[i].score < maxstep) break;
		if (v[i].score > maxstep)
		{
			maxstep = v[i].score;
		}
		best_choice.push_back(v[i].id);
	}
	return true;
}








void Find(int x, int y, int i, int j, int index, int board[9][9], char block[9][9], char block_Qi[81], char pos[81][2], char grid[9][9], char record[9][9])
{
	//(dx,dy)Ϊ�գ������� 
    if (board[i][j] == 0)
    {
        switch (block_Qi[index])
        {  
        //ԭ��û��������������1�� 
        case 0:
            pos[index][0] = i;
            pos[index][1] = j;
            block_Qi[index] = 1;
            if (board[x][y] == 1)	//���Լ��� 
            {
                grid[i][j] |= 2;
            }
            else	//�ǶԷ��� 
            {
                grid[i][j] |= 1;
                if (record[i][j] == -1 && board[x][y] == -1) 
                {
                    record[i][j] = index;
                }
            }
            break;
        //ԭ����1���������ڲ�ֹ1�� 
        case 1:               
			//ͳ���ظ���                                                              
            if (pos[index][0] == i && pos[index][1] == j)
            {
                break;
            }
            block_Qi[index] = 2;
            //�Լ�����������������Ψһ���� 
            if (board[x][y] == 1)
            {
                grid[(int)pos[index][0]][(int)pos[index][1]] |= 4;
                grid[i][j] |= 4;
            }
            //�����ǶԷ�Ψһ����clear 1 
            else if (record[(int)pos[index][0]][(int)pos[index][1]] == index) 
            {
                grid[(int)pos[index][0]][(int)pos[index][1]] = grid[(int)pos[index][0]][(int)pos[index][1]] / 2 * 2;
                record[(int)pos[index][0]][(int)pos[index][1]] = -1;
            }
            break;
        //ԭ���Ͳ�ֹ1��������Ҳ��ֹ 
        case 2:
            if (board[x][y] == 1)
            {
                grid[i][j] |= 4;
            }
            //���ظ��¶Է���Ψһ��� 
            break;
        }
    }
}

//�ݹ������ÿ��group��������������ÿ�����ӵ����������ÿһ��group��������� 
void Pre(int x, int y, int index, int board[9][9], char block[9][9], char block_Qi[81], char pos[81][2], char grid[9][9], char record[9][9])
{
	block[x][y] = index;
	int tx,ty;
	for(int i = 0; i < 4; i++)
	{
		tx = x+dxdy[i][0];
		ty = y+dxdy[i][1];
		//����(x,y)��(tx,ty)��� 
		if(tx>=0&&ty>=0&&tx<=8&&ty<=8)
			Find(x, y, tx, ty, index, board, block, block_Qi, pos, grid, record);
		//ͬɫ���������� 
		if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&block[tx][ty]==-1&&board[x][y]==board[tx][ty])
			Pre(tx, ty, index, board, block, block_Qi, pos, grid, record);
	}
}


//����ÿ��λ���ж��ٲ�����ֵ�����ز�����������result�����¼�����µĸ�������������� 
int CountBu(int board[9][9], int result[9][9])
{
    int r = 0;
    int groupCount = 0;
    char block[9][9];    //blockIndex 
    char record[9][9]; 	//����ǶԷ���Ψһ�����������¼�Ա���ѯ���� 
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
            record[i][j] = -1;
            block[i][j] = -1;
        }
    }
    char block_Qi[81] = {0}; 	//block����   
    char pos[81][2] = {0}; 		//ǰ������λ��  
    char grid[9][9] = {0};     //�����λ�ֱ��ʾ�Է�1��������1����������1�� 
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
        	//�ջ����ѷ��飬���� 
            if (block[i][j] != -1 || board[i][j] == 0)
            {
                continue;
            }
            Pre(i, j, groupCount++, board, block, block_Qi, pos, grid, record);
        }
    }
    
    for (int i = 0; i < 9; i++)
    {
        for (int j = 0; j < 9; j++)
        {
        	//������ ������ 
            if (board[i][j] != 0)
            {
                result[i][j] = 0;
                continue;
            }
            //�Է�Ψһ�� ���Ϸ� ������ 
            if (grid[i][j] % 2 == 1) 
            {
                result[i][j] = 0;
                continue;
            }
            //�����µ�λ�� 
            if (grid[i][j] / 4 == 1) 
            {
                result[i][j] = 1;
                r++;
                continue;
            }
            //����Ψһ����Ҫ�������Ժ��Ƿ����� 
            if (grid[i][j] / 2 % 2 == 1) 
            {
                if ((i != 0 && board[i - 1][j] == 0) || (j != 0 && board[i][j - 1] == 0) || (i != 8 && board[i + 1][j] == 0) || (j != 8 && board[i][j + 1] == 0))
                {
                    result[i][j] = 1;
                    r++;
                }
                //������ 
                else
                {
                    result[i][j] = 0;
                }
                continue;
            }
            //���涼��Χ�����µĵ㣬��˵����жϣ���Ϊ֮ǰͳ����ʱδͳ�Ƶ������ 
            if ((i == 0 || board[i - 1][j] == -1) && (j == 0 || board[i][j - 1] == -1) && (i == 8 || board[i + 1][j] == -1) && (j == 8 || board[i][j + 1] == -1))
            {
                result[i][j] = 0;
                continue; 
            }
            //�����κ�һ������ ������ 
            result[i][j] = 1;
            r++;
        }
    }
    return r;
}



class MCT
{
public:
    MCT *parent;	//parent node
    MCT *children[MCTSn];	//children node list
    int board[9][9];		//temp board
    int Move[MCTSn][2]; 	//Next move axis
    int child;             	//���
    int maxn;           	//���ʵĸ�������
    int value;            	//�����ʵĵ÷֣�UCB�㷨
    int visn;            	//��ǰ���ʼ���������UCB�㷨       
    double score;           //��ǰ����ֵ�÷�
    int *times;             //���ʵĵ������      
	
	//constructor
    MCT(){}
    MCT(int pre_board[9][9], int nex[2], MCT *p, int *countp):value(0),visn(0),child(0)
    {
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                board[i][j] = -pre_board[i][j]; 
            }
        }
        board[nex[0]][nex[1]] = -1; 
        parent = p;
        times = countp;
        best_child(); 	//����չ���͸�����;���
    }
    
    //ѡ����õ�һ�� 
    MCT *Best()
    {
        if (maxn == 0) return this;
        if (maxn > child) 
        {
            MCT *c = new MCT(board, Move[child], this, times);
            children[child++] = c;
            return c;
        }

        int bestChild = 0;
        double maxScore = 0;
        for (int i = 0; i < child; i++)
        {
            if (maxScore < children[i]->score)
            {
                bestChild = i;
                maxScore = children[i]->score;
            }
        }
        return children[bestChild]->Best();
    }

	//ģ��������һ�������Ӯ�ĸ��ʲ�ת��Ϊ���� 
    int Expand()
    {
        int neg_board[9][9]; 
        int result[9][9];

        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                neg_board[i][j] = -board[i][j];
            }
        }
        double a = (double)CountBu(board, result);
        double b = (double)CountBu(neg_board, result);
        double rate;
        if (a == b)
        {
            rate = 0.5;
        }
        else
        {
			if(a>b) rate=1.0-0.5/(a-b+1.0);
			else rate=0.5/(b-a+1.0);
        }
        if (rate * (double)RAND_MAX >= (double)rand())
        {
            return 1;
        }
        else
        {
            return -1;
        }
    }

	//��Ҷ�ӽ���score���򴫲�ֱ���� 
    void backup(int flag)
    {
        MCT *node = this;
        int d = 0;
        while (node != NULL)	//ROOT
        {
            if (flag == d % 2) 
            {
                node->value++;
            }
            node->visn++;
            //UCB��ֵ
            node->score = double(node->value)/double(node->visn)+C*sqrt(2*log(double(*times))/double(node->visn)); 
            node = node->parent;
            d++;
        }
    }

	//������+��MinMax���
    void best_child()
    {
        int result[9][9];
        int resultTemp[9][9];
        int neg_board[9][9]; 
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                neg_board[i][j] = -board[i][j];
            }
        }
        int sump = CountBu(board, result);
        int choice[81];
        int visn = 0;
        for (int i = 0; i < 9; i++)
        {
            for (int j = 0; j < 9; j++)
            {
                if (result[i][j])
                {
                    choice[visn] = 9 * i + j;
                    visn++;
                }
            }
        }

        if (sump <= MCTSn)
        {
            maxn = sump;
            for (int i = 0; i < sump; i++)
            {
                Move[i][0] = choice[i]/9;
                Move[i][1] = choice[i]%9;
            }
        }
        else
        {
            maxn = MCTSn;
            int positionMark[81]; 	//score
            for (int i = 0; i < sump; i++)
            {
                int x, y;
                x = choice[i] / 9;
                y = choice[i] % 9;
                int score = 0; 
				int tx,ty,ttx,tty;
				//��������Χ�۵�˼·�����мƷ֣�Χ�ۼӷ֣�����۷�
				for(int m = 0; m < 4; m++)
				{
					ttx = x+dxdy[m][0], tty = y+dxdy[m][1];
					tx = x+exey[m][0], ty = y+exey[m][1];
					if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&board[tx][ty]!=0) score++;
					if(tx>=0&&ty>=0&&tx<=8&&ty<=8&&board[tx][ty]==1) score--;
				}

                board[x][y] = 1;
                neg_board[x][y] = -1;
				//����MinMax˼·���ñ������ߵĲ�����ȥ�Է����ߵĲ��������Լ��ĵ÷� 
                score += CountBu(board, resultTemp)*4;
                score -= CountBu(neg_board, resultTemp)*4;

                board[x][y] = 0;
                neg_board[x][y] = 0;

                positionMark[i] = score;
            }
			//ð�ݣ����յ÷ִ�С�ݼ�����ÿ��move 
            for (int i = 0; i < MCTSn; i++)
            {
                int temp = positionMark[i];
                int tempi = i;
                for (int j = i; j < sump; j++)
                {
                    if (temp < positionMark[j])
                    {
                        temp = positionMark[j];
                        tempi = j;
                    }
                }
                swap(positionMark[i], positionMark[tempi]);
                swap(choice[i], choice[tempi]);
            }
            for (int i = 0; i < MCTSn; i++)
            {
                Move[i][0] = choice[i] / 9;
                Move[i][1] = choice[i] % 9;
            }
        }
    }



};



void BGM()
{
	play_music = !play_music;
	if (play_music)
	{
		mciSendString("open ./bgm1.mp3 alias BGM", 0, 0, 0);
		mciSendString("play BGM repeat", 0, 0, 0);
	}
	else
	{
		mciSendString("close BGM", 0, 0, 0);
	}
}

void ShowHelp()
{
	cleardevice();
	IMAGE rule;
	loadimage(&rule, "./rule.jpg", WID, HEI, true);
	putimage(0, 0,&rule);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle(WID-100, HEI-100, WID-20, HEI-20);
	settextcolor(BLACK);
	settextstyle(30, 20, "����");
	char ss[] = "����";
	int wid = 80 / 2 - textwidth(ss) / 2;
	int hei = 80 / 2 - textheight(ss) / 2;
	outtextxy(wid + WID-100, hei + HEI-100, ss);
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > WID-100 && mmsg.x < WID-20 && mmsg.y>HEI-100 && mmsg.y < HEI-20)
			{
				clock_t stclock = clock();
				cout << "������������ҳ" << endl;
				setfillcolor(RGB(218, 165, 32));
				setlinecolor(BLACK);
				fillrectangle(WID - 100, HEI - 100, WID - 20, HEI - 20);
				outtextxy(wid + WID - 100, hei + HEI - 100, ss);
				while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
				return;
			}
		}
	}
}

inline void drawAlpha(IMAGE* picture, int  picture_x, int picture_y) //xΪ����ͼƬ��X���꣬yΪY����
{

	// ������ʼ��
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()���������ڻ�ȡ��ͼ�豸���Դ�ָ�룬EASYX�Դ�
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //��ȡpicture���Դ�ָ��
	int picture_width = picture->getwidth(); //��ȡpicture�Ŀ�ȣ�EASYX�Դ�
	int picture_height = picture->getheight(); //��ȡpicture�ĸ߶ȣ�EASYX�Դ�
	int graphWidth = getwidth();       //��ȡ��ͼ���Ŀ�ȣ�EASYX�Դ�
	int graphHeight = getheight();     //��ȡ��ͼ���ĸ߶ȣ�EASYX�Դ�
	int dstX = 0;    //���Դ������صĽǱ�

	// ʵ��͸����ͼ ��ʽ�� Cp=��p*FP+(1-��p)*BP �� ��Ҷ˹���������е���ɫ�ĸ��ʼ���
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //���Դ������صĽǱ�
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA��͸����
			int sr = ((src[srcX] & 0xff0000) >> 16); //��ȡRGB���R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //���Դ������صĽǱ�
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //��ʽ�� Cp=��p*FP+(1-��p)*BP  �� ��p=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //��p=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //��p=sa/255 , FP=sb , BP=db
			}
		}
	}
}


void JudgeRes()
{
	if (is_over == 0) return;
	HWND hnd = GetHWnd();
	if (is_over == 1)
	{
		int choice = MessageBox(hnd, "�ڷ�ʤ��", "��ʾ", MB_RETRYCANCEL | MB_ICONWARNING);
		if (choice == IDRETRY)
		{
			cout << "�ڷ�ʤ��" << endl;
			SetWindowText(hnd, "��Χ��");
			int cc = MessageBox(hnd, "�Ƿ����¿��֣�", "��ʾ", MB_YESNO);
			if (cc == IDYES)
			{
				cout << "���¿���" << endl;
				return;
			}
			else
			{
				exit(0);
			}
		}
		exit(0);
	}
	else
	{
		int choice = MessageBox(hnd, "�׷�ʤ��", "��ʾ", MB_RETRYCANCEL | MB_ICONWARNING);
		if (choice == IDRETRY)
		{
			cout << "�׷�ʤ��" << endl;
			SetWindowText(hnd, "��Χ��");
			int cc = MessageBox(hnd, "�Ƿ����¿��֣�", "��ʾ", MB_YESNO);
			if (cc == IDYES)
			{
				cout << "���¿���" << endl;
				return;
			}
			else
			{
				exit(0);
			}
		}
		exit(0);
	}
}

int main()
{
	play_music = false;
	player_color = 0;
	initgraph(WID, HEI,SHOWCONSOLE);
	setbkcolor(WHITE);
	IMAGE startbg,voice,novoice;
	loadimage(&novoice, "./novoice.jpg", 60, 60, true);
	loadimage(&voice, "./voice.jpg", 60, 60, true);
	loadimage(&startbg, "./startbg.jpg",WID,HEI,true);
startloc:
	putimage(0, 0, &startbg);
	if (play_music == true)putimage(780, 640, &voice);
	else putimage(780, 640, &novoice);
	setbkmode(TRANSPARENT);
	settextcolor(BLACK);
	settextstyle(80, 50, "���Ĳ���");
	char sb[] = "��Χ��";
	int wid = WID / 2 - textwidth(sb)/2;
	outtextxy(wid, 100, sb);
	settextstyle(60, 40, "Times New Roman");
	char sb2[] = "NOGO";
	wid = WID / 2 - textwidth(sb2)/2;
	outtextxy(wid, 200, sb2);

	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle(WID/2-100, 300, WID/2+100, 350);	//����
	fillrectangle(WID/2-100, 380, WID/2+100, 430);	//�˻�
	fillrectangle(WID/2-100, 460, WID/2+100, 510);	//˫��
	fillrectangle(WID/2-100, 540, WID/2+100, 590);	//�˳�
	settextcolor(BLACK);
	settextstyle(30, 20, "����");
	char s1[] = "��Ϸ����";
	char s2[] = "�˻���ս";
	char s3[] = "˫�˶�ս";
	char s4[] = "�˳���Ϸ";
	int width = 200 / 2 - textwidth(s1) / 2;
	int height = 50 / 2 - textheight(s1) / 2;
	outtextxy(width + WID/2-100, height + 300, s1);
	outtextxy(width + WID/2-100, height + 380, s2);
	outtextxy(width + WID/2-100, height + 460, s3);
	outtextxy(width + WID/2-100, height + 540, s4);
	HWND hnd = GetHWnd();

	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG msg = GetMouseMsg();
			if (msg.uMsg==WM_LBUTTONDOWN)
			{
				if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>300 && msg.y < 350)
				{
					clock_t stclock = clock();
					cout << "����ѡ��" << endl;
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 300, WID / 2 + 100, 350);
					outtextxy(width + WID / 2 - 100, height + 300, s1);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					ShowHelp();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>380 && msg.y < 430)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 380, WID / 2 + 100, 430);
					outtextxy(width + WID / 2 - 100, height + 380, s2);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "�˻���սģʽ" << endl;
					memset(Board, 0, sizeof(Board));
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					int co = MessageBox(hnd, "�Ƿ�ѡ��ڷ���", "ѡ�������ɫ", MB_YESNO);
					if (co == IDYES)
					{
						player_color = 0;
						Color = -1;
					}
					else
					{
						player_color = 1;
						Color = 1;
					}
					OnePlay();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>460 && msg.y < 510)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 460, WID / 2 + 100, 510);
					outtextxy(width + WID / 2 - 100, height + 460, s3);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "˫�˶�սģʽ" << endl;
					memset(Board, 0, sizeof(Board));
					memset(actionR, 0, sizeof(actionR));
					TwoPlay();
					goto startloc;
				}
				else if (msg.x > WID/2-100 && msg.x < WID/2+100 && msg.y>540 && msg.y < 590)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(BLACK);
					fillrectangle(WID / 2 - 100, 540, WID / 2 + 100, 590);
					outtextxy(width + WID / 2 - 100, height + 540, s4);
					while (clock() - stclock < (double)100000 / CLOCKS_PER_SEC) {};
					cout << "�˳�" << endl;
					return 0;
				}
				else if (msg.x > 780 && msg.x < 840 && msg.y>640 && msg.y < 700)
				{
					BGM();
					if (play_music == true)putimage(780, 640, &voice);
					else putimage(780, 640, &novoice);
				}
			}
		}
	}
	closegraph();
}

void OnePlay()
{
	is_over = 0;
	cleardevice();
	IMAGE boardimg,whiteimg,voice,novoice;
	setbkmode(TRANSPARENT);
	loadimage(&novoice, "./novoice.jpg", 40, 40, true);
	loadimage(&whiteimg, "./white.jpg", WID - HEI, HEI, true);
	loadimage(&boardimg, "./board1.jpg", HEI, HEI, true);
	loadimage(&voice, "./voice.jpg", 40, 40, true);
	BeginBatchDraw();
	putimage(WID - HEI, 0, &boardimg);
	putimage(0, 0, &whiteimg);
	if (play_music == true)putimage(20, HEI-80, &voice);
	else putimage(20, HEI-80, &novoice);
	settextstyle(40, 38, "����");
	settextcolor(BLACK);
	char ss[] = "������";
	int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
	outtextxy(wid, 40, ss);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
	fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
	fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
	fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
	fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
	fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

	settextstyle(30, 30, "����");
	char s0[] = "����";
	char s1[] = "����";
	char s2[] = "�浵";
	char s3[] = "����";
	char s4[] = "����";
	char s5[] = "�˳�";
	wid = 120 / 2 - textwidth(s1) / 2;
	int hei = 70 / 2 - textheight(s1) / 2;
	int tp = (WID - HEI) / 2 - 60;
	outtextxy(tp + wid, 210 + hei, s0);
	outtextxy(tp + wid, 300 + hei, s1);
	outtextxy(tp + wid, 390 + hei, s2);
	outtextxy(tp + wid, 480 + hei, s3);
	outtextxy(tp + wid, 570 + hei, s4);
	outtextxy(tp + wid, 660 + hei, s5);
	EndBatchDraw();
	IMAGE bchess, wchess;
	loadimage(&wchess, "./����.png", 74, 74, true);
	loadimage(&bchess, "./����.png", 74, 74, true);
firstlast:
	if (player_color == 1)
	{
		Play();
		FlushMouseMsgBuffer();
		mutex = true;
		if (is_over)
		{
			cout << is_over << endl;
			return;
		}
	}
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>tp&&mmsg.x<tp+120)
			{
				if (mmsg.y > 210 && mmsg.y < 280)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225,225,220));
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					outtextxy(tp + wid, 210 + hei, s0);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ�����" << endl;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					EndBatchDraw();
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					memset(Board, 0, sizeof(Board));
					HWND hnd = GetHWnd();
					MessageBox(hnd, "���¿��֣�", "��ʾ", MB_OK);
					goto firstlast;
				}
				else if (mmsg.y>300 && mmsg.y < 370)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					outtextxy(tp + wid, 300 + hei, s1);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ�����" << endl;
					Board[actionR[0]][actionR[1]] = 0;
					Board[AIactionR[0]][AIactionR[1]] = 0;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();

				}
				else if (mmsg.y > 390 && mmsg.y < 460)
				{
					clock_t stclock = clock();
					Save();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ��浵" << endl;
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
				}
				else if (mmsg.y > 480 && mmsg.y < 550)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					outtextxy(tp + wid, 480 + hei, s3);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ�����" << endl;
					Load();
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					HWND hnd = GetHWnd();
					if (player_color == 0)
					{
						MessageBox(hnd, "�����ɫΪ��ɫ", "������ʾ", MB_OK);
					}
					else
					{
						MessageBox(hnd, "�����ɫΪ��ɫ", "������ʾ", MB_OK);
					}
					is_over = 0;
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
				}
				else if (mmsg.y > 570 && mmsg.y < 640)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					outtextxy(tp + wid, 570 + hei, s4);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ�����" << endl;
					HWND hnd = GetHWnd();
					int cho = MessageBox(hnd, "�Ƿ����䣿","����", MB_YESNO);
					if (cho == IDYES)
					{
						MessageBox(hnd, "������䣬����ʤ����", "��Ϸ����", MB_OK);
						mutex = true;
						return;
					}
					else
					{
						setfillcolor(YELLOW);
						setlinecolor(WHITE);
						fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
						outtextxy(tp + wid, 570 + hei, s4);
					}
				}
				else if (mmsg.y > 660 && mmsg.y < 730)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);
					outtextxy(tp + wid, 660 + hei, s5);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ��˳�" << endl;
					exit(0);
				}
			}
			else if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > 20 && mmsg.x<60 && mmsg.y>HEI - 80 && mmsg.y < HEI - 40)
			{
				BGM();
				if (play_music == true)putimage(20, HEI - 80, &voice);
				else putimage(20, HEI - 80, &novoice);
			}

			//�����������
			else if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>WID-HEI)
			{
				if (mutex == false) continue;
				mutex = false;
				cout << "mutex!" << endl;
				int tx, ty;
				pair<int, int> tpa=Choose_cross(mmsg.x,mmsg.y);
				tx = (&tpa)->first, ty = (&tpa)->second;
				cout << tx << " " << ty << endl;
				if (Board[tx][ty] != 0)
				{
					HWND hwd = GetHWnd();
					int chong = MessageBox(hwd, "��λ���������ӣ�", "��ʾ", MB_OK);
					mutex = true;
					continue;
				}
				if (player_color == 0) drawAlpha(&bchess,cross_posx[tx] - 37, cross_posy[ty] - 37);
				else drawAlpha(&wchess, cross_posx[tx] - 37, cross_posy[ty] - 37);
				
				if (!ProcStep(tx, ty, -Color))	//���Ϸ�
				{
					mutex = true;
					if (player_color)
					{
						cout <<"case 1"<< endl;
						is_over = 1;
						JudgeRes();
						return;
					}
					else
					{
						cout << "case 2" << endl;
						is_over = -1;
						JudgeRes();
						return;
					}
				}
				else
				{
					if (CountBu1(Color))
					{
						if (player_color)
						{
							cout << "case 3" << endl;
							is_over = -1;
							JudgeRes();
							return;
						}
						else
						{
							cout << "case 4" << endl;
							is_over = 1;
							JudgeRes();
							return;
						}
					}
				}
				Board[tx][ty] = (player_color ? -1 : 1);
				actionR[0] =tx, actionR[1]=ty;
				if (player_color == 0)
				{
					Play();
					FlushMouseMsgBuffer();
					mutex = true;
					if (is_over)
					{
						cout << is_over << endl;
						return;
					}
					goto firstlast;
				}
				else goto firstlast;
			}
			
		}
	}
}


inline pair<int, int> Choose_cross(int x, int y)
{
	int dx=1000, dy=1000;
	int xx, yy;
	for (int i = 0; i < 9; i++)
	{
		int tp = abs(x - cross_posx[i]);
		if (tp < dx)
		{
			xx = i;
			dx = tp;
		}
		tp = abs(y - cross_posy[i]);
		if (tp < dy)
		{
			yy = i;
			dy = tp;
		}
	}
	return make_pair(xx, yy);
}


void TwoPlay()
{
	is_over = 0;
	cleardevice();
	IMAGE boardimg, whiteimg, voice, novoice;
	setbkmode(TRANSPARENT);
	loadimage(&novoice, "./novoice.jpg", 40, 40, true);
	loadimage(&whiteimg, "./white.jpg", WID - HEI, HEI, true);
	loadimage(&boardimg, "./board1.jpg", HEI, HEI, true);
	loadimage(&voice, "./voice.jpg", 40, 40, true);
	BeginBatchDraw();
	putimage(WID - HEI, 0, &boardimg);
	putimage(0, 0, &whiteimg);
	if (play_music == true)putimage(20, HEI - 80, &voice);
	else putimage(20, HEI - 80, &novoice);
	settextstyle(40, 38, "����");
	settextcolor(BLACK);
	char ss[] = "������";
	int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
	outtextxy(wid, 40, ss);
	setlinestyle(PS_SOLID, 5);
	setfillcolor(YELLOW);
	setlinecolor(WHITE);
	fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
	fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
	fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
	fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
	fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
	fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

	settextstyle(30, 30, "����");
	char s0[] = "����";
	char s1[] = "����";
	char s2[] = "�浵";
	char s3[] = "����";
	char s4[] = "����";
	char s5[] = "�˳�";
	wid = 120 / 2 - textwidth(s1) / 2;
	int hei = 70 / 2 - textheight(s1) / 2;
	int tp = (WID - HEI) / 2 - 60;
	outtextxy(tp + wid, 210 + hei, s0);
	outtextxy(tp + wid, 300 + hei, s1);
	outtextxy(tp + wid, 390 + hei, s2);
	outtextxy(tp + wid, 480 + hei, s3);
	outtextxy(tp + wid, 570 + hei, s4);
	outtextxy(tp + wid, 660 + hei, s5);
	EndBatchDraw();
	IMAGE bchess, wchess;
	loadimage(&wchess, "./����.png", 74, 74, true);
	loadimage(&bchess, "./����.png", 74, 74, true);
	curcolor = 1;		//�Ӻڷ���ʼ����
	while (1)
	{
		if (MouseHit())
		{
			MOUSEMSG mmsg = GetMouseMsg();
			if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > tp && mmsg.x < tp + 120)
			{
				if (mmsg.y > 210 && mmsg.y < 280)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					outtextxy(tp + wid, 210 + hei, s0);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "˫�ˣ�����" << endl;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					EndBatchDraw();
					memset(actionR, 0, sizeof(actionR));
					memset(AIactionR, 0, sizeof(AIactionR));
					memset(Board, 0, sizeof(Board));
					HWND hnd = GetHWnd();
					MessageBox(hnd, "���¿��֣�", "��ʾ", MB_OK);
					curcolor = 1;
				}
				else if (mmsg.y > 300 && mmsg.y < 370)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					outtextxy(tp + wid, 300 + hei, s1);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "˫�ˣ�����" << endl;
					Board[actionR[0]][actionR[1]] = 0;
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					curcolor *= -1;

				}
				else if (mmsg.y > 390 && mmsg.y < 460)
				{
					clock_t stclock = clock();
					Save2();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "���ˣ��浵" << endl;
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					outtextxy(tp + wid, 390 + hei, s2);
				}
				else if (mmsg.y > 480 && mmsg.y < 550)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					outtextxy(tp + wid, 480 + hei, s3);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "˫�ˣ�����" << endl;
					Load2();
					cleardevice();
					BeginBatchDraw();
					putimage(WID - HEI, 0, &boardimg);
					putimage(0, 0, &whiteimg);
					putimage(20, HEI - 80, &voice);
					settextstyle(40, 38, "����");
					settextcolor(BLACK);
					int wid = (WID - HEI) / 2 - textwidth(ss) / 2;
					outtextxy(wid, 40, ss);
					setlinestyle(PS_SOLID, 5);
					setfillcolor(YELLOW);
					setlinecolor(WHITE);
					fillrectangle((WID - HEI) / 2 - 60, 210, (WID - HEI) / 2 + 60, 280);
					fillrectangle((WID - HEI) / 2 - 60, 300, (WID - HEI) / 2 + 60, 370);
					fillrectangle((WID - HEI) / 2 - 60, 390, (WID - HEI) / 2 + 60, 460);
					fillrectangle((WID - HEI) / 2 - 60, 480, (WID - HEI) / 2 + 60, 550);
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);

					settextstyle(30, 30, "����");
					wid = 120 / 2 - textwidth(s1) / 2;
					int hei = 70 / 2 - textheight(s1) / 2;
					int tp = (WID - HEI) / 2 - 60;
					outtextxy(tp + wid, 210 + hei, s0);
					outtextxy(tp + wid, 300 + hei, s1);
					outtextxy(tp + wid, 390 + hei, s2);
					outtextxy(tp + wid, 480 + hei, s3);
					outtextxy(tp + wid, 570 + hei, s4);
					outtextxy(tp + wid, 660 + hei, s5);
					for (int i = 0; i < 9; i++)
					{
						for (int j = 0; j < 9; j++)
						{
							if (Board[i][j] == 1)
							{
								drawAlpha(&bchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
							else if (Board[i][j] == -1)
							{
								drawAlpha(&wchess, cross_posx[i] - 37, cross_posy[j] - 37);
							}
						}
					}
					EndBatchDraw();
					HWND hnd = GetHWnd();
					if (curcolor == 1)
					{
						MessageBox(hnd, "�ֵ��ڷ�����", "������ʾ", MB_OK);
					}
					else
					{
						MessageBox(hnd, "�ֵ��׷�����", "������ʾ", MB_OK);
					}
					is_over = 0;
					memset(actionR, 0, sizeof(actionR));
				}
				else if (mmsg.y > 570 && mmsg.y < 640)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
					outtextxy(tp + wid, 570 + hei, s4);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "˫�ˣ�����" << endl;
					HWND hnd = GetHWnd();
					int cho = MessageBox(hnd, "�Ƿ����䣿", "����", MB_YESNO);
					if (cho == IDYES)
					{
						if (curcolor == 1)
						{
							MessageBox(hnd, "�ڷ����䣬�׷�ʤ����", "��Ϸ����", MB_OK);
						}
						else
						{
							MessageBox(hnd, "�׷����䣬�ڷ�ʤ����", "��Ϸ����", MB_OK);
						}
						return;
					}
					else
					{
						setfillcolor(YELLOW);
						setlinecolor(WHITE);
						fillrectangle((WID - HEI) / 2 - 60, 570, (WID - HEI) / 2 + 60, 640);
						outtextxy(tp + wid, 570 + hei, s4);
					}
				}
				else if (mmsg.y > 660 && mmsg.y < 730)
				{
					clock_t stclock = clock();
					setfillcolor(RGB(218, 165, 32));
					setlinecolor(RGB(225, 225, 220));
					fillrectangle((WID - HEI) / 2 - 60, 660, (WID - HEI) / 2 + 60, 730);
					outtextxy(tp + wid, 660 + hei, s5);
					while (clock() - stclock < (double)200000 / CLOCKS_PER_SEC) {};
					cout << "˫���˳�" << endl;
					exit(0);
				}
			}
			else if (mmsg.uMsg == WM_LBUTTONDOWN && mmsg.x > 20 && mmsg.x<60 && mmsg.y>HEI - 80 && mmsg.y < HEI - 40)
			{
				BGM();
				if (play_music == true)putimage(20, HEI - 80, &voice);
				else putimage(20, HEI - 80, &novoice);
			}
			//�����������
			else if (mmsg.uMsg == WM_LBUTTONDOWN&&mmsg.x>WID-HEI)
			{
				int tx, ty;
				pair<int, int> tpa = Choose_cross(mmsg.x, mmsg.y);
				tx = (&tpa)->first, ty = (&tpa)->second;
				cout << tx << " " << ty << endl;
				if (Board[tx][ty] != 0)
				{
					HWND hwd = GetHWnd();
					int chong = MessageBox(hwd, "��λ���������ӣ�", "��ʾ", MB_OK);
					continue;
				}
				if (curcolor == 1) drawAlpha(&bchess, cross_posx[tx] - 37, cross_posy[ty] - 37);
				else drawAlpha(&wchess, cross_posx[tx] - 37, cross_posy[ty] - 37);

				if (!ProcStep(tx, ty, curcolor))	//���Ϸ�
				{
					if (curcolor==-1)
					{
						cout << "case 1" << endl;
						is_over = 1;
						JudgeRes();
						return;
					}
					else
					{
						cout << "case 2" << endl;
						is_over = -1;
						JudgeRes();
						return;
					}
				}
				else
				{
					if (CountBu1(-curcolor))
					{
						if (curcolor==-1)
						{
							cout << "case 3" << endl;
							is_over = -1;
							JudgeRes();
							return;
						}
						else
						{
							cout << "case 4" << endl;
							is_over = 1;
							JudgeRes();
							return;
						}
					}
				}
				Board[tx][ty] = curcolor;
				actionR[0] = tx, actionR[1] = ty;
				curcolor *= -1;
			}

		}
	}
}



void Play()
{
	int p_count = 0;
	srand((unsigned)time(0));
	int MCTSt = (int)(0.9 * (double)CLOCKS_PER_SEC);
	int board[9][9];
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (Color == 1)
			{
				board[i][j] = -Board[i][j];
			}
			else
			{
				board[i][j] = Board[i][j];
			}
		}
	}

	MCT root(board, actionR, NULL, &p_count);

	int start = clock();
	while (clock() - start < MCTSt)
	{
		p_count++;
		MCT* node = root.Best();
		int result = node->Expand();
		node->backup(result == 1 ? 1 : 0); //traceback
	}

	int max = 0;
	int maxI = 0;
	int* bestAction = root.Move[0];
	for (int i = 0; i < root.child; i++)
	{
		if (max < root.children[i]->visn)
		{
			maxI = i;
			max = root.children[i]->visn;
			bestAction = root.Move[i];
		}
	}
	int ax = bestAction[0], ay = bestAction[1];
	IMAGE bchess, wchess;
	loadimage(&wchess, "./����.png", 74, 74, true);
	loadimage(&bchess, "./����.png", 74, 74, true);
	cout << "AI" <<" "<<ax<<" "<<ay<<endl;
	if (player_color == 0) drawAlpha(&wchess, cross_posx[ax] - 37, cross_posy[ay] - 37);
	else drawAlpha(&bchess, cross_posx[ax] - 37, cross_posy[ay] - 37);
	if (!ProcStep(ax, ay, Color))	//���Ϸ�
	{
		if (player_color)
		{
			is_over = -1;
			JudgeRes();
			return;
		}
		else
		{
			is_over = 1;
			JudgeRes();
			return;
		}
	}
	else
	{
		if (CountBu1(-Color))
		{
			if (player_color)
			{
				is_over = 1;
				JudgeRes();
				return;
			}
			else
			{
				is_over = -1;
				JudgeRes();
				return;
			}
		}
	}
	Board[ax][ay] = (player_color ? 1 : -1);
	AIactionR[0] = ax, AIactionR[1] = ay;
	return;
}


void Save()
{
	ofstream outfile("archiveSingle.out");
	if (outfile.is_open())
	{
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (Board[i][j] == 1)
				{
					outfile << "b";
				}
				else if (Board[i][j] == -1)
				{
					outfile << "w";
				}
				else outfile << "o";
			}
		}
		outfile.close();
	}
	else
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
}

void Save2()
{
	ofstream outfile("archiveDouble.out");
	if (outfile.is_open())
	{
		for (int i = 0; i < 9; i++)
		{
			for (int j = 0; j < 9; j++)
			{
				if (Board[i][j] == 1)
				{
					outfile << "b";
				}
				else if (Board[i][j] == -1)
				{
					outfile << "w";
				}
				else outfile << "o";
			}
		}
		outfile.close();
	}
	else
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
}
void Load()
{
	int wcount, bcount;
	wcount = bcount = 0;
	ifstream infile("archiveSingle.out");
	if (!infile.is_open())
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
	char s[100];
	infile.getline(s, 99);
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (s[i * 9 + j] == 'w')
			{
				Board[i][j] = -1;
				wcount++;
			}
			else if (s[i * 9 + j] == 'b')
			{
				Board[i][j] = 1;
				bcount++;
			}
			else Board[i][j] = 0;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << Board[i][j] << " ";
		}
		cout << endl;
	}
	if (wcount == bcount)
	{
		player_color = 0;
		Color = -1;
	}
	else
	{
		player_color = 1;
		Color = 1;
	}
	cout << "��һ�������ɫΪ";
	cout << (player_color ? "��ɫ" : "��ɫ") << endl;
}

void Load2()
{
	int wcount, bcount;
	wcount = bcount = 0;
	ifstream infile("archiveDouble.out");
	if (!infile.is_open())
	{
		cerr << "Error opening file" << endl;
		exit(1);
	}
	char s[100];
	infile.getline(s, 99);
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			if (s[i * 9 + j] == 'w')
			{
				Board[i][j] = -1;
				wcount++;
			}
			else if (s[i * 9 + j] == 'b')
			{
				Board[i][j] = 1;
				bcount++;
			}
			else Board[i][j] = 0;
		}
	}
	for (int i = 0; i < 9; i++)
	{
		for (int j = 0; j < 9; j++)
		{
			cout << Board[i][j] << " ";
		}
		cout << endl;
	}
	if (wcount == bcount)
	{
		curcolor = 1;
	}
	else
	{
		curcolor = -1;
	}
	cout << "�ֵ�";
	cout << (curcolor==-1 ? "�׷�" : "�ڷ�") << "������"<<endl;
}
