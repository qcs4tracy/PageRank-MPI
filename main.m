x = [2 3 4 5 6 7 8];
A = load('data.txt');
y1 = A(:,1)';
y2 = A(:,2)';
semilogy(x,y1,'-bo;MPI-PR;',x,y2,'-kx;SEQ-PR;');
title('MPI vs Sequential PageRank');
xlabel('# of processes');
ylabel('time');
print -deps graph.eps
