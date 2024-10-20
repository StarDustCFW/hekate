// Variables para manejar bloques de control
int if_condition = 0; // 0 = no condición activa, 1 = condición verdadera, -1 = falsa

void lineHandler(char line[])
{
    // Ignorar comentarios
    if (line[0] == '#')
        return;

    // Manejar instrucciones de impresión
    if (line[0] == '[' || line[0] == '.')
    {
        if (if_condition != -1)  // Solo imprime si la condición es verdadera o no hay if
        {
            memmove(line, line + 1, strlen(line));
            printerCU("", line, 0);
        }
        return;
    }

    // Condicional if: verificar si un archivo existe
    if (strncmp(line, "if ", 3) == 0)
    {
        memmove(line, line + 3, strlen(line));
        if (sd_file_exists(line))
        {
            if_condition = 1;  // Condición verdadera
        }
        else
        {
            if_condition = -1;  // Condición falsa
        }
        return;
    }

    // else: cambiar el valor de la condición
    if (strncmp(line, "else", 4) == 0)
    {
        if_condition = (if_condition == 1) ? -1 : 1; // Cambia de verdadera a falsa, y viceversa
        return;
    }

    // Ejecutar las demás instrucciones solo si la condición es verdadera o no hay if
    if (if_condition != -1)
    {
        // Copiar al clipboard
        if (line[0] == '<')
        {
            memmove(line, line + 1, strlen(line));
            clip = line;
            return;
        }

        // Pegar lo del clipboard
        if (line[0] == '>')
        {
            if (sd_file_exists(clip))
            {
                memmove(line, line + 1, strlen(line));
                copyfile(clip, line);
            }
            return;
        }

        // Mover lo del clipboard
        if (line[0] == '~')
        {
            memmove(line, line + 1, strlen(line));
            f_rename(clip, line);
            return;
        }

        // Establecer filtro
        if (line[0] == '*')
        {
            memmove(line, line + 1, strlen(line));
            type = line;
            return;
        }

        // Crear carpeta o archivo
        if (line[0] == '+')
        {
            memmove(line, line + 1, strlen(line));
            if (line[strlen(line) - 1] == '/')
            {
                line[strlen(line) - 1] = 0;
                f_mkdir(line);
                return;
            }
            printerCU(line, "", 0);
            sd_save_2_file(clip, strlen(clip), line);
            return;
        }

        // Eliminar archivos o carpetas
        if (line[0] == '-')
        {
            memmove(line, line + 1, strlen(line));
            if (line[strlen(line) - 1] == '/')
            {
                line[strlen(line) - 1] = 0;
                deleteall(line, type, "");
                return;
            }
            f_unlink(line);
            return;
        }
    }
}

// Ejemplo de uso en el script
/*
# Este es un ejemplo de como usar if y else
if /path/al/archivo
> /destino/del/archivo
else
- /ruta/diferente/
*/
