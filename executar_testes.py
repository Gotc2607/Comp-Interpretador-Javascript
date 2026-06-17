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
    arquivo_err = arquivo_js.replace('.js', '.err')
    arquivo_rc = arquivo_js.replace('.js', '.rc')

    try:
        with open(arquivo_js, 'r') as f_in:
            resultado = subprocess.run(
                [EXECUTAVEL], stdin=f_in, capture_output=True, text=True, timeout=5
            )

        saida_obtida = resultado.stdout.strip()
        erro_obtido = resultado.stderr.strip()
        rc_obtido = resultado.returncode

        saida_esperada = ''
        if os.path.exists(arquivo_gabarito):
            with open(arquivo_gabarito, 'r') as f_out:
                saida_esperada = f_out.read().strip()

        erro_esperado = ''
        if os.path.exists(arquivo_err):
            with open(arquivo_err, 'r') as f_err:
                erro_esperado = f_err.read().strip()

        rc_esperado = 0
        if os.path.exists(arquivo_rc):
            with open(arquivo_rc, 'r') as f_rc:
                rc_esperado = int(f_rc.read().strip())

        passou = True

        if saida_obtida != saida_esperada:
            passou = False
            print(f"{C_FAIL}[FAIL]{C_RESET} {arquivo_js}")
            print(f"       stdout esperado: '{saida_esperada}'")
            print(f"       stdout obtido:   '{saida_obtida}'")

        if erro_obtido != erro_esperado:
            if passou:
                print(f"{C_FAIL}[FAIL]{C_RESET} {arquivo_js}")
                passou = False
            print(f"       stderr esperado: '{erro_esperado}'")
            print(f"       stderr obtido:   '{erro_obtido}'")

        if rc_obtido != rc_esperado:
            if passou:
                print(f"{C_FAIL}[FAIL]{C_RESET} {arquivo_js}")
                passou = False
            print(f"       return code esperado: {rc_esperado}")
            print(f"       return code obtido:   {rc_obtido}")

        if passou:
            print(f"{C_PASS}[PASS]{C_RESET} {arquivo_js}")
        return passou

    except Exception as e:
        print(f"{C_FAIL}[ERROR]{C_RESET} Falha na execucao de {arquivo_js}: {e}")
        return False

def main():
    if not os.path.exists(EXECUTAVEL):
        print(f"Erro Fatal: Executavel '{EXECUTAVEL}' nao encontrado. Compile primeiro.")
        sys.exit(1)
        
    # Usando os.walk para buscar arquivos .js recursivamente (incluindo subpastas)
    arquivos_js = []
    for root, dirs, files in os.walk(DIRETORIO_TESTES):
        for file in files:
            if file.endswith(".js"):
                # Guarda o caminho completo do arquivo
                arquivos_js.append(os.path.join(root, file))
                
    arquivos_js.sort()
    
    total = len(arquivos_js)
    passou = 0

    print(f"Executando suite de testes ({total} casos de teste)...\n")

    for js in arquivos_js:
        
        gabarito = js.replace(".js", ".out")
        
        if executar_teste(js, gabarito):
            passou += 1

    print(f"\nResumo: {passou}/{total} testes passaram.")
    if passou < total:
        sys.exit(1)

if __name__ == "__main__":
    main()