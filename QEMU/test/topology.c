#include <stdio.h>
#include <string.h>

#define N 100005
int edge[N], head[N], next[N], size = 0;
int n, m, in[N], que[N], h = 0, t = -1;

void add(int start, int end) {
    printf("adding edges %d -->> %d\n", start, end);
    edge[size] = end; next[size] = head[start]; head[start] = size++;    
    printf("added edges %d -->> %d\n", start, end);
}

bool top_sort() {
    for (int i = 1; i <= n; ++i) {
        if (in[i] == 0) que[++t] = i;
    }
    while (h <= t) {
        int cur = que[h++];
        for (int i = head[cur]; i != -1; i = next[i]) {
            int nbr = edge[i];
            in[nbr]--;
            if (in[nbr] == 0) {
                que[++t] = nbr;
            }
        }
    }
    return h == n;
}

int main() {
    scanf("%d%d", &n, &m);
    int a, b;
    memset(head, -1, sizeof head);
    for (int i = 0; i < m; ++i) {
        scanf("%d%d", &a, &b);
        add(a, b);
        in[b]++;
    }
    if (top_sort()) {
        for (int i = 0; i < h; ++i) {
            printf("%d ", que[i]);
        }
    }
    else printf("-1");
    return 0;
}
