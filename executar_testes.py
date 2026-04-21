import subprocess
import os
import sys

DIRETORIO_TESTES = "testes"
EXECUTAVEL = "./interpretador"

C_PASS = '\033[92m'
C_FAIL = '\033[91m'
C_WARN = '\033[93m'
C_RESET = '\033[0m'

def executar_teste(arquivo_js, arquivo_gabarito):
    try:
        with open(arquivo_js, 'r') as f_in:
            resultado = subprocess.run(
                [EXECUTAVEL], stdin=f_in, capture_output=True, text=True, timeout=5
            )
        
        saida_obtida = resultado.stdout.strip()

        with open(arquivo_gabarito, 'r') as f_out:
            saida_esperada = f_out.read().strip()

        if saida_obtida == saida_esperada:
            print(f"{C_PASS}[PASS]{C_RESET} {arquivo_js}")
            return True
        else:
            print(f"{C_FAIL}[FAIL]{C_RESET} {arquivo_js}")
            print(f"       Esperado: '{saida_esperada}'")
            print(f"       Obtido:   '{saida_obtida}'")
            return False
            
    except Exception as e:
        print(f"{C_FAIL}[ERROR]{C_RESET} Falha na execucao de {arquivo_js}: {e}")
        return False

def main():
    if not os.path.exists(EXECUTAVEL):
        print(f"Erro Fatal: Executavel '{EXECUTAVEL}' nao encontrado. Compile primeiro.")
        sys.exit(1)
        
    arquivos = [f for f in os.listdir(DIRETORIO_TESTES) if f.endswith(".js")]
    arquivos.sort()
    
    total = len(arquivos)
    passou = 0

    print(f"Executando suite de testes ({total} casos de teste)...\n")

    for arq in arquivos:
        js = os.path.join(DIRETORIO_TESTES, arq)
        gabarito = os.path.join(DIRETORIO_TESTES, arq.replace(".js", ".out"))
        
        if executar_teste(js, gabarito):
            passou += 1

    print(f"\nResumo: {passou}/{total} testes passaram.")
    if passou < total:
        sys.exit(1)

if __name__ == "__main__":
    main()