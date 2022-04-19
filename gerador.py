import numpy as np
float_formatter = "{:.2f}".format

def gerar_n_com_soma_v(proportion, n, v):
    retorno = np.zeros(n, dtype = int)
    
    for i in range(n):
        retorno[i] = v * proportion[i]/np.sum(proportion)

    return retorno


def gerador(m, n,
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
    
    
    with open('your_file.txt', 'w') as f:
        f.write("%d " % n)
        f.write("%d " % T)
        f.write("%d " % W)
        f.write("%d\n" % m)
        f.write("%d\n" % Cw)
        for l in range(m):
            for i in range(card_IL[l]):
                f.write("%d " % calI[l][i])
            f.write("\n")
            
        for l in range(m):
            for i in range(card_IL[l]):
                f.write("%d " % q[l][i])
            f.write("\n")
        
        for l in range(m):
            for t in range(T):
                f.write("%d " % CP[l][t])
            f.write("\n")
        
        for l in range(m):
            for i in range(card_IL[l]):
                f.write("%.4f " % p[l][i])
            f.write("\n")
            
        for i in range(n):
            f.write("%d " % Iplus[i])
        f.write("\n")
        
        for i in range(n):
            f.write("%d " % Iminus[i])
        f.write("\n\n")
        
        for i in range(n):
            for t in range(T):
                f.write("%d " % d[i][t])
            f.write("\n")
        
        for l in range(m):
            for i in range(card_IL[l]):
                for j in range(card_IL[l]):
                    f.write("%d " % e[l][i][j])
                f.write("\n")
            f.write("\n")
        
        for i in range(n):
            f.write("%.4f " % h[i])
        f.write("\n")
        
        for i in range(n):
            f.write("%.4f " % g[i])
        f.write("\n")

        
        for l in range(m):
            for i in range(card_IL[l]):
                f.write("%.4f " % cp[l][i])
            f.write("\n")
            

            
        for l in range(m):
            for i in range(card_IL[l]):
                for j in range(card_IL[l]):
                    f.write("%d " % cs[l][i][j])
                f.write("\n")
            f.write("\n")

gerador(2,8,2,6,
        10000,14000,
        5, 8,
        1, 9,
        0.0012,0.04,
        4000, 500,
        9000, 13000,
        0.27, 0.54,
        10, 15,
        0.8,1.2,
        80, 100);