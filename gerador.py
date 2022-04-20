import numpy as np
float_formatter = "{:.2f}".format

def gerar_n_com_soma_v(proportion, n, v):
    retorno = np.zeros(n, dtype = int)
    
    for i in range(n):
        retorno[i] = np.round(v * proportion[i]/np.sum(proportion))

    return retorno


def gerador(instance, m, n,
            e_low, e_high,
            Cw_low, Cw_high,
            IL_low, IL_high,
            q_low, q_high,
            p_low, p_high,
            Iplus_max, Iminus_max,
            d_min, d_max,
            h_min, h_max,
            g_min, g_max,
            cp_min, cp_max,
            cs_min, cs_max):
    T = 16
    W = 7*16
    Cw = np.random.randint(Cw_low, Cw_high);
    

    card_IL = np.random.randint(IL_low, IL_high, size=(m))
    
    calI = []
    turnos = []
    p = []
    q = []
    
    for i in range(m):
        calI.append(list(np.random.choice(np.arange(1,n+1),size = (card_IL[i]), replace=False) ))
        turnos.append(list(np.random.randint(q_low, q_high, size =(card_IL[i]))))
        p.append(list(np.random.uniform(p_low,p_high, size =(card_IL[i])).round(4)))
        q.append(np.zeros(card_IL[i]))
    
    q = np.asarray(q, dtype = "object")
    for l in range(m):
        for i in range(card_IL[l]):
            q[l][i] = np.round(8*turnos[l][i]/p[l][i])
    
    CP = np.full((m,T), 160);
    
    Iplus = np.random.randint(0,Iplus_max,size = n)
    Iminus = np.random.randint(0, Iminus_max, size = n)

    for i in range(n):
        if Iminus[i] < Iplus[i]:
            Iminus[i] = 0
            Iplus[i] = Iplus[i] - Iminus[i]
        elif Iminus[i] > Iplus[i]:
            Iplus[i] = 0
            Iminus[i] = Iminus[i] - Iplus[i]
        else:
            Iplus[i] = 0
            Iminus[i] = 0

    soma = np.sum(Iplus)
    if(soma > Cw)
        for i in range(n):
            Iplus[i] = np.round(Iplus[i]/soma * Cw)
#    
#    excedente = soma - Cw
#    if  excedente < 0:
#        excedente = excedente/np.count_nonzero()
#        for i in range(n):
#            Iplus[i] -= excedente
    
    dd = np.random.randint(d_min, d_max, size = T)
    
    d = []
    
    proportion = np.random.uniform(0.05,0.9,size = n)
    
    for t in range(T):
        d.append(gerar_n_com_soma_v(proportion, n, dd[t]))
    
    d = np.asarray(d).transpose()
    
    e = []
    for l in range(m):
        ee = np.random.randint(e_low, e_high, size =(card_IL[l],card_IL[l] ))
        for i in range(card_IL[l]):
            ee[i][i] = 0
        e.append(ee)
        
    e = np.asarray(e,dtype="object")
    
    h = np.random.uniform(h_min, h_max, size = n).round(4)
    g = np.zeros(n)
    for i in range(n):
        g[i] = np.random.uniform(g_min, g_max, size = 1) * h[i]
        g[i] = g[i].round(4)
    


    cp = []
    
    for l in range(m):
        vec_ax = np.zeros(card_IL[l])
        for i in range(card_IL[l]):
            vec_ax[i] = np.random.uniform(cp_min, cp_max, size = 1) * float(p[l][i])
        cp.append(vec_ax)
    cp = np.asarray(cp,dtype="object")
    
    media = 0.0
    for l in range(m):
        for i in range(card_IL[l]):
            for j in range(card_IL[l]):
                media += e[l][i][j]
    
    media /= (m* card_IL[l] * card_IL[l] )
    
    cs = []
    for l in range(m):
        ccss = np.random.randint(cs_min, cs_max, size =(card_IL[l],card_IL[l] ))
        for i in range(card_IL[l]):
            for j in range(card_IL[l]):
                ccss[i][j] *= e[l][i][j]/media
        cs.append(ccss)
        
    cs = np.asarray(cs,dtype="object")
    
    
    with open(instance, 'w') as f:
        f.write("%d " % n)
        f.write("%d " % T)
        f.write("%d " % W)
        f.write("%d\n" % m)
        f.write("%d\n" % Cw)
        for l in range(m):
            for i in range(card_IL[l]):
                if i == card_IL[l] - 1:
                    f.write("%d" % calI[l][i])
                else:
                    f.write("%d " % calI[l][i])
            f.write("\n")
            
        for l in range(m):
            for i in range(card_IL[l]):
                if i == card_IL[l] - 1:
                    f.write("%d" % q[l][i])
                else:
                    f.write("%d " % q[l][i])
            f.write("\n")
        
        for l in range(m):
            for t in range(T):
                if t == T - 1:
                    f.write("%d" % CP[l][t])
                else:
                    f.write("%d " % CP[l][t])
            f.write("\n")
        
        for l in range(m):
            for i in range(card_IL[l]):
                if i == card_IL[l] - 1:
                    f.write("%.4f" % p[l][i])
                else:
                    f.write("%.4f " % p[l][i])
            f.write("\n")
            
        for i in range(n):
            if i == n - 1:
                f.write("%d" % Iplus[i])
            else:
                f.write("%d " % Iplus[i])
        f.write("\n")
        
        for i in range(n):
            if i == n - 1:
                f.write("%d" % Iminus[i])
            else:
                f.write("%d " % Iminus[i])
        f.write("\n")
        
        for i in range(n):
            for t in range(T):
                if t == T - 1:
                    f.write("%d" % d[i][t])
                else:
                    f.write("%d " % d[i][t])
            f.write("\n")
        
        for l in range(m):
            for i in range(card_IL[l]):
                for j in range(card_IL[l]):
                    if j == card_IL[l] - 1:
                        f.write("%d" % e[l][i][j])
                    else:
                        f.write("%d " % e[l][i][j])
                f.write("\n")
            if i != card_IL[l] - 1:
                f.write("\n")
        
        for i in range(n):
            if i != n - 1:
                f.write("%.4f " % h[i])
            else:
                f.write("%.4f" % h[i])
        f.write("\n")
        
        for i in range(n):
            if i != n - 1:
                f.write("%.4f " % g[i])
            else:
                f.write("%.4f" % g[i])
        f.write("\n")

        
        for l in range(m):
            for i in range(card_IL[l]):
                if i != card_IL[l] - 1:
                    f.write("%.4f " % cp[l][i])
                else:
                    f.write("%.4f" % cp[l][i])
            f.write("\n")
            

            
        for l in range(m):
            for i in range(card_IL[l]):
                for j in range(card_IL[l]):
                    if j != card_IL[l] - 1:
                        f.write("%d " % cs[l][i][j])
                    else:
                        f.write("%d" % cs[l][i][j])
                f.write("\n")
            if i != card_IL[l] - 1:
                f.write("\n")

#Grupo A
instancias = ['A6.txt', 'A7.txt', 'A8.txt', 'A9.txt', 'A10.txt']
for inst in instancias:
    gerador(inst,2,8,2,6,
            10000,14000,
            5, 8,
            1, 9,
            0.012,0.04,
            4000, 500,
            9000, 13000,
            0.27, 0.54,
            10, 15,
            0.8,1.2,
            80, 100);

#Grupo B
instancias = ['B6.txt', 'B7.txt', 'B8.txt', 'B9.txt', 'B10.txt']
for inst in instancias:
	gerador(inst,3,12,5,9,
	        14000,18000,
	        3, 9,
	        3, 6,
	        0.008,0.05,
	        4000, 500,
	        16000, 24000,
	        0.2, 0.34,
	        10, 15,
	        0.8,1.2,
	        100, 200);

#Grupo C
instancias = ['C6.txt', 'C7.txt', 'C8.txt', 'C9.txt', 'C10.txt']
for inst in instancias:
	gerador(inst,4,16,2,6,
	        40000,48000,
	        4, 10,
	        3, 6,
	        0.007,0.017,
	        20000, 2000,
	        19000, 78000,
	        0.3, 0.87,
	        10, 15,
	        0.8,1.2,
	        100, 200);

#Grupo D
instancias = ['D6.txt', 'D7.txt', 'D8.txt', 'D9.txt', 'D10.txt']
for inst in instancias:
	gerador(inst,5,20,1,6,
	        180000,220000,
	        5, 12,
	        6, 9,
	        0.003,0.01,
	        50000, 4000,
	        27000, 220000,
	        0.07, 0.21,
	        10, 15,
	        0.8,1.2,
	        230, 1200);

#Grupo E
instancias = ['E6.txt', 'E7.txt', 'E8.txt', 'E9.txt', 'E10.txt']
for inst in instancias:
	gerador(inst,7,28,2,8,
	        120000,150000,
	        2, 12,
	        3, 6,
	        0.005,0.028,
	        30000, 4000,
	        65000, 100000,
	        0.087, 0.433,
	        10, 15,
	        0.8,1.2,
	        150, 620);