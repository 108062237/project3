#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <cmath>
#define INF 2147483647
struct Point {
    int x, y;
	Point() : Point(0, 0) {}
	Point(float x, float y) : x(x), y(y) {}
	bool operator==(const Point& rhs) const {
		return x == rhs.x && y == rhs.y;
	}
	bool operator!=(const Point& rhs) const {
		return !operator==(rhs);
	}
	Point operator+(const Point& rhs) const {
		return Point(x + rhs.x, y + rhs.y);
	}
	Point operator-(const Point& rhs) const {
		return Point(x - rhs.x, y - rhs.y);
	}
};
const int SIZE = 8;
class state {
public:
    enum SPOT_STATE {
        EMPTY = 0,
        BLACK = 1,
        WHITE = 2
    };
    static const int SIZE = 8;
    const std::array<Point, 8> directions{{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1), /*{0, 0}, */Point(0, 1),
        Point(1, -1), Point(1, 0), Point(1, 1)
    }};
    std::array<std::array<int, SIZE>, SIZE> board;
    std::vector<Point> next_valid_spots;
    std::array<int, 3> disc_count;
    int cur_player;
    bool done;
    int winner;
public:
    state() {
        reset();
    }
    state(const state &tmp){
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = tmp.board[i][j];
            }
        }
        for(auto c:tmp.next_valid_spots){
            next_valid_spots.push_back(c);
        }
        disc_count[0] = tmp.disc_count[0];
        disc_count[1] = tmp.disc_count[1];
        disc_count[2] = tmp.disc_count[2];

        cur_player = tmp.cur_player;
        done = tmp.done;
        winner = tmp.winner;
    }
    int get_next_player(int player) const {
        return 3 - player;
    }
    bool is_spot_on_board(Point p) const {
        return 0 <= p.x && p.x < SIZE && 0 <= p.y && p.y < SIZE;
    }
    int get_disc(Point p) const {
        return board[p.x][p.y];
    }
    void set_disc(Point p, int disc) {
        board[p.x][p.y] = disc;
    }
    bool is_disc_at(Point p, int disc) const {
        if (!is_spot_on_board(p))
            return false;
        if (get_disc(p) != disc)
            return false;
        return true;
    }
    bool is_spot_valid(Point center) const {
        if (get_disc(center) != EMPTY)
            return false;
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player))
                    return true;
                p = p + dir;
            }
        }
        return false;
    }
    void flip_discs(Point center) {
        for (Point dir: directions) {
            // Move along the direction while testing.
            Point p = center + dir;
            if (!is_disc_at(p, get_next_player(cur_player)))
                continue;
            std::vector<Point> discs({p});
            p = p + dir;
            while (is_spot_on_board(p) && get_disc(p) != EMPTY) {
                if (is_disc_at(p, cur_player)) {
                    for (Point s: discs) {
                        set_disc(s, cur_player);
                    }
                    disc_count[cur_player] += discs.size();
                    disc_count[get_next_player(cur_player)] -= discs.size();
                    break;
                }
                discs.push_back(p);
                p = p + dir;
            }
        }
    }
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = EMPTY;
            }
        }
        board[3][4] = board[4][3] = BLACK;
        board[3][3] = board[4][4] = WHITE;
        cur_player = BLACK;
        disc_count[EMPTY] = 8*8-4;
        disc_count[BLACK] = 2;
        disc_count[WHITE] = 2;
        next_valid_spots = get_valid_spots();
        done = false;
        winner = -1;
    }
    void update(){
        disc_count[EMPTY] = 0;
        disc_count[BLACK] = 0;
        disc_count[WHITE] = 0;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                disc_count[board[i][j]] ++;
            }
        }
    }
    std::vector<Point> get_valid_spots() const {
        std::vector<Point> valid_spots;
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                Point p = Point(i, j);
                if (board[i][j] != EMPTY)
                    continue;
                if (is_spot_valid(p))
                    valid_spots.push_back(p);
            }
        }
        return valid_spots;
    }
    bool put_disc(Point p) {
        if(!is_spot_valid(p)) {
            winner = get_next_player(cur_player);
            done = true;
            return false;
        }
        set_disc(p, cur_player);
        disc_count[cur_player]++;
        disc_count[EMPTY]--;
        flip_discs(p);
        // Give control to the other player.
        cur_player = get_next_player(cur_player);
        next_valid_spots = get_valid_spots();
        // Check Win
        if (next_valid_spots.size() == 0) {
            cur_player = get_next_player(cur_player);
            next_valid_spots = get_valid_spots();
            if (next_valid_spots.size() == 0) {
                // Game ends
                done = true;
                int white_discs = disc_count[WHITE];
                int black_discs = disc_count[BLACK];
                if (white_discs == black_discs) winner = EMPTY;
                else if (black_discs > white_discs) winner = BLACK;
                else winner = WHITE;
            }
        }
        return true;
    }

};

//------------------------------------------------------------------------------------------

state cur;
void read_board(std::ifstream& fin) {

    fin >> cur.cur_player;
    for (int i = 0; i < SIZE; i++) {
        for (int j = 0; j < SIZE; j++) {
            fin >> cur.board[i][j];
        }
    }

}

void read_valid_spots(std::ifstream& fin) {
    int n_valid_spots;
    fin >> n_valid_spots;
    int x, y;
    cur.next_valid_spots.clear();
    for (int i = 0; i < n_valid_spots; i++) {
        fin >> x >> y;
        cur.next_valid_spots.push_back({x, y});
    }
    cur.update();
}



const int boardscore[8][8] =
{
	{ 500, -70, 50,  20,  20, 50,  -70,  500 },
	{-70, -100,-10, -10, -10,-10, -100, -70 },
	{ 50,  -10, 10,   0,   0, 10,  -10,  50 },
	{ 20,  -10,  0,   0,   0,  0,  -10,  20 },
	{ 20,  -10,  0,   0,   0,  0,  -10,  20 },
	{ 50,  -10, 10,   0,   0, 10,  -10,  50 },
	{-70, -100,-10, -10, -10,-10, -100, -70 },
	{ 500, -70, 50,  20,  20, 50,  -70,  500 }
};
int heuristic(Point p,state cur_state){

    int h=boardscore[p.x][p.y];
    if((p.x==1&&p.y==1)||(p.x==0&&p.y==1)||(p.x==1&&p.y==0)){
        if(cur_state.board[0][0]==cur_state.cur_player)
            h*=-1;
    }
    if((p.x==6&&p.y==0)||(p.x==6&&p.y==1)||(p.x==7&&p.y==1)){
        if(cur_state.board[7][0]==cur_state.cur_player)
            h*=-1;
    }
    if((p.x==0&&p.y==6)||(p.x==1&&p.y==6)||(p.x==1&&p.y==7)){
        if(cur_state.board[0][7]==cur_state.cur_player)
            h*=-1;
    }
    if((p.x==6&&p.y==6)||(p.x==7&&p.y==6)||(p.x==6&&p.y==7)){
        if(cur_state.board[7][7]==cur_state.cur_player)
            h*=-1;
    }
    Point dir[4]{
        Point(-1, -1), Point(-1, 0), Point(-1, 1),
        Point(0, -1)
    };
    int s=0;
    int a1=cur_state.disc_count[3-cur_state.cur_player];
    cur_state.put_disc(p);

    int a2=cur_state.disc_count[cur_state.cur_player];
    if(cur_state.disc_count[cur_state.EMPTY]<=5){
        h+=(a1-a2)*30;
    }
    else if(cur_state.disc_count[cur_state.EMPTY]<=10){
        h+=(a1-a2)*10;
    }
    else if(cur_state.disc_count[cur_state.EMPTY]<=20){
        h+=(a1-a2)*5;
    }
    else {
        h+=(a1-a2)*3;
    }
    h-=cur_state.next_valid_spots.size()*15;
    //cur現在是enemy
    for(int i=0;i<8;i++){
        for(int j=0;j<8;j++){

            if(cur_state.board[i][j]==cur_state.EMPTY)
                continue;
            Point tmp(i,j);
            int score=0;
            for(int k=0;k<4;k++){
                int n=0;
                int l=0,r=0;//1=wall 2=empty 3=enemy
                while(cur_state.is_spot_on_board(tmp)&&cur_state.board[i][j]==cur_state.cur_player){
                        tmp=tmp+dir[k];
                }
                if(!cur_state.is_spot_on_board(tmp)){
                    l=1;
                }
                else if(cur_state.board[tmp.x][tmp.y]==cur_state.cur_player){
                    l=3;
                }
                else {
                    l=2;
                }
                tmp=tmp-dir[k];
                while(cur_state.is_spot_on_board(tmp)&&cur_state.board[i][j]==cur_state.cur_player){
                    tmp=tmp-dir[k];
                    n++;
                }
                if(!cur_state.is_spot_on_board(tmp)){
                    r=1;
                }
                else if(cur_state.board[tmp.x][tmp.y]==cur_state.cur_player){
                    r=3;
                }
                else {
                    r=2;
                }
                if(l==1&&r==1){
                    score+=5;
                }
                else if(l==1||r==1){
                    score+=1;
                }
                else if((l==2&&r==3)||(l==3&&r==2)){
                    score-=2;
                }

            }
            if(cur_state.board[i][j]==cur_state.cur_player)
                h-=score;
            else
                h+=score;

        }
    }

    return h;
}
int AB_pruning(state cur_state,int depth,bool maximizing,Point p,int alpha,int beta){ //maximizing =1 找最大 =0找最小
    if(depth==0){
        int h;
        if(cur_state.cur_player!=cur.cur_player){
            return -heuristic(p,cur_state);
        }
        else{
            return heuristic(p,cur_state);
        }
    }
    state next_state(cur_state);
    next_state.put_disc(p);

    if(maximizing){
        for(auto c:next_state.next_valid_spots){
            int h=AB_pruning(next_state,depth-1,!maximizing,c,alpha,beta);
            if(h>alpha){
                alpha=h;
            }
            if(alpha>=beta)
                break;
        }

        return alpha;
    }
    else if(!maximizing){
        for(auto c:next_state.next_valid_spots){
            int h=AB_pruning(next_state,depth-1,maximizing,c,alpha,beta);
            if(h<beta){
                beta=h;
            }
            if(alpha>=beta)
                break;
        }
        return beta;
    }


}
void write_valid_spot(std::ofstream& fout) {
    /*int n_valid_spots = next_valid_spots.size();
    srand(time(NULL));
    // Keep updating the output until getting killed.
    while (true) {
        // Choose random spot. (Not random uniform here)
        int index = (rand() % n_valid_spots);
        Point p = next_valid_spots[index];
        // Remember to flush the output to ensure the last action is written to file.
        fout << p.x << " " << p.y << std::endl;
        fout.flush();
    }*/
    Point p;
    p.x=9,p.y=9;
    int heuristic_maxm=-INF,m=-INF;
    int alpha=-INF,beta=INF;
    for(auto c:cur.next_valid_spots){
        int h=heuristic(c,cur);
        if(h>m){
            fout << c.x << " " << c.y << std::endl;
            fout.flush();
            m=h;
        }
    }
    for(auto c:cur.next_valid_spots){

        int h=AB_pruning(cur,5,0,c,alpha,beta);

        if(h>heuristic_maxm){
            heuristic_maxm=h;
            alpha=h;
            p=c;
        }

    }
    //std::cout<<heuristic_maxm<<" "<<p.x<<" "<<p.y<<std::endl;

    fout << p.x << " " << p.y << std::endl;
        fout.flush();
}

int main(int, char** argv) {
    std::ifstream fin(argv[1]);
    std::ofstream fout(argv[2]);
    read_board(fin);
    read_valid_spots(fin);
    write_valid_spot(fout);
    fin.close();
    fout.close();
    return 0;
}
