#ifndef _ENCRYPT_H_
#define _ENCRYPT_H_


///////////////////////////////////////////////////////////////////////////////
/// Simple enough - call it twice to get your data back
inline void Encrypt(TCHAR *inp, DWORD inplen, const TCHAR* key = NULL, 
												DWORD keylen = 0)
///////////////////////////////////////////////////////////////////////////////
{
#ifndef _DEBUG
	if (IsDebuggerPresent())
	{
		FatalExit(0);
		throw;
	}
#endif


    // We will consider size of sbox 256 bytes
    // (extra byte are only to prevent any bugs just in case)
    TCHAR Sbox[257], Sbox2[257];
    unsigned long i, j, t, x;

	if (!key)
	{   
		key = _T("Rain, rain, go away");
		keylen = (DWORD)_tcslen(key);
	}


    TCHAR temp , k = 0;
    i = j = t =  x = 0;
    temp = 0;

    // Always initialize the arrays with zero
    ZeroMemory(Sbox, sizeof(Sbox));
    ZeroMemory(Sbox2, sizeof(Sbox2));

    // Initialize sbox i
    for(i = 0; i < 256U; i++)
    {
        Sbox[i] = (TCHAR)i;
    }

    j = 0;
    // Initialize the sbox2 with key
    for(i = 0; i < 256U ; i++)
    {
        if(j == keylen)
        {
            j = 0;
        }
        Sbox2[i] = key[j++];
    }    

    j = 0 ; //Initialize j
    // Scramble sbox1 with sbox2
    for(i = 0; i < 256; i++)
    {
        j = (j + (unsigned long) Sbox[i] + (unsigned long) Sbox2[i]) % 256U ;
        temp =  Sbox[i];                    
        Sbox[i] = Sbox[j];
        Sbox[j] =  temp;
    }

    i = j = 0;
    for(x = 0; x < inplen; x++)
    {
        // Increment i
        i = (i + 1U) % 256U;
        // Increment j
        j = (j + (unsigned long) Sbox[i]) % 256U;

        // Scramble SBox #1 further so encryption routine will
        // will repeat itself at great interval
        temp = Sbox[i];
        Sbox[i] = Sbox[j] ;
        Sbox[j] = temp;

        // Get ready to create pseudo random  byte for encryption key
        t = ((unsigned long) Sbox[i] + (unsigned long) Sbox[j]) %  256U ;

        // Get the random byte
        k = Sbox[t];

        // Xor with the data and done
        inp[x] = (inp[x] ^ k);
    }    
}

#endif