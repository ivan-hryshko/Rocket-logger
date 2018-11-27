import java.io.*;
import java.io.File;
import java.nio.file.StandardOpenOption;
import java.util.ArrayList;
import java.util.List;
import java.nio.file.Files;
import java.nio.file.Paths;


import static java.lang.Integer.getInteger;
import static java.lang.Integer.parseInt;

public class Translator {

    //Что будем преобравывать
    public static String pathsStart = "d:/data.bin";
    //Куда будем преобразвывать
    public static String pathsFinish = "d:/Test.csv";


    //Измерим время роботы программы
    public static long startTime = System.currentTimeMillis();

    public static  int SymbolNum = 1;

    private static List<String> hexs = new ArrayList<String>();
    private static ArrayList<Integer> ValuesFinal = new ArrayList<>();

    public static int[] buffer  = new int[32];
    private static String [] StringHexVal = new String[32];
    private static String  [] FinalBuffer = new String[18];

    public static int []  FinalBufferInt = new int[FinalBuffer.length];

    public static int [][] FinalBufferToFix = new int[30][FinalBufferInt.length];

    public static boolean checkTempBMPflag = false;
    public static boolean ReadyToWriteflag = false;
    public static int clearIter = 0;
    public static  int bufferIterator = 0;

    public static String StringToWrite = "";

    public static ArrayList<int[]> correctArr = new ArrayList<int[]>();





    public static void main(String[] args) {

        //Записываем в конечный файл подисиси к столбцам и размерности
        WriteStartToFile();



        //Читать  файл в буффер по 32 байта
        File file = new File(pathsStart);        try (FileInputStream fis = new FileInputStream(file)) {
            System.out.println("Total file size to read (in bytes) : "+ fis.available());

            int readedByte;
            byte NumberReadByte=0;

            //Пропустить первый стартовый 1кб при запуске, там только
            int skipCharsFromBegin = 1024;
            int charNum = skipCharsFromBegin;
                        while ((readedByte = fis.read()) != -1 && charNum > 1)
            {
                charNum--;
                SymbolNum++;
            }

            //Считываем в буффер по 32 байта
            while ((readedByte = fis.read()) != -1) {

                //Наполняем буффер, когда 32 байт, то отправляем на преобразование
                System.out.print("Read from file " +readedByte+" ");

                if(ReadyToWriteflag){
                    System.out.println();
                    System.out.println("i'm here");
                    //SoutToFix();

                    //
//                    ToString(FinalBufferInt);

                    ToStringFinaBufferToFix();

                    //Дописываем в файл, преобразованный буффер
                    WriteToFile();

                    ClearToFixBuffer();
                    ReadyToWriteflag = false;
                    System.out.println();
                    System.out.println("Write");
                    NumberReadByte =0;
                    buffer[NumberReadByte]=readedByte;




                }
                else if(NumberReadByte==31){

                    buffer[NumberReadByte]=readedByte;
                    System.out.print("Add to buffer "+buffer[NumberReadByte]+" " + NumberReadByte);
                    System.out.println();


                    // Если 0, 30 и 31 байт ff - значит пойшли отформатированные данные и все хорошо, на этом можно заверништь программу
                    if(buffer[0]==255&&buffer[31]==255){

                        System.out.println();
                        System.out.println("DATA END CORRECT");
                        return;
                    }

                    //если проверяющие символы не такие как ожидаются, досрочно завершаеи программу
                    else if(buffer[0]!='['&&buffer[31]!=']'){
                        System.out.println();
                        System.out.print("FAIL!!!  Symbol Num: ");
                        System.out.println(SymbolNum);
                        TakeTime();
                        for (int i = 0; i <buffer.length ; i++) {
                            System.out.print(buffer[0]+" ");
                        }

                        return;
                    }


                    //Превращаем буффер из Инт в Стринг
                    ToStringHex(buffer);

                    //вывести то что получилось
                    outHexListOneDim(StringHexVal);

                    //Проверяем, чтобы все элементы массива после преоброзвания состояли из двух символов
                    //Это касается чисел от 0 до 9 они могут быть только позитывными, а значит в конце дополняем "0"
                    CheckSize(StringHexVal);

                    //Основные шаги преобразования, тут решаем что делать с каждым полученым байтов
                    //Формируем финальный Хекс массив
                    FormedFinalHexBuffer(StringHexVal);

                    //Преобразовываем из 16ной в 10-ную систему
                    FinalBufferInt = ConvertToDec(FinalBuffer);

                    //outHexListOneDim(FinalBuffer);

                    //Перобразовываем сырые данные с датчиков в реальные показания.
                    MathOper(FinalBufferInt);

                    //
                    SendToFix(FinalBufferInt);



                    //Наша песня хороша
                    SymbolNum+=32;
                    NumberReadByte=0;

                }

                //Наполняем буффер, когда 32 байт, то отправляем на преобразование
                else {
                    buffer[NumberReadByte]=readedByte;
                    System.out.print("Add to buffer "+buffer[NumberReadByte]+" " + NumberReadByte);
                    System.out.println();
                    NumberReadByte++;
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }


        TakeTime();




//          Не трогай, второй вариант и пример на случай если забудешь
//        try {
//            HexVal = get2DimHexArray(bytesfromFile);
//        } catch (IOException e) {
//            e.printStackTrace();
//        }
//        System.out.println(HexVal[0][0]);

        //Вывести двумерный на экран
//        outHexListOneDim(StringHexVal);

//        String some =  HexVal[0][0].substring(0, 1);
//        System.out.println(  some);

//        HexVal = HexCutMetode(HexVal);
//        outHexList(HexVal);
//        System.out.println();

//        for(String s : hexs){
//        ValuesFinal.add(            Integer.parseUnsignedInt(s, 16));
//        }


        outList(ValuesFinal);
        int y = (int) Long.parseLong("80000000", 16);
        System.out.println(y);
        System.out.println();
//        int x =Integer.toHexString("FFFF");
//        System.out.println(x);


        //Проверяем что считало
        System.out.println(hexs.toString());




    }

    private static void SoutToFix() {
        for (int [] x: FinalBufferToFix){
            System.out.println(x[9]);
        }
    }

    private static void ClearToFixBuffer() {
        //Записываем впервую строку финального буфера, то значение на котором закончилось бмп
        int NumberToClearFromStarFinalBuffer = 0;
        FinalBufferToFix[NumberToClearFromStarFinalBuffer]=FinalBufferToFix[clearIter];
        clearIter++;
        NumberToClearFromStarFinalBuffer++;

        while (FinalBufferToFix[clearIter][10]!=0){
            FinalBufferToFix[NumberToClearFromStarFinalBuffer]=FinalBufferToFix[clearIter];
            clearIter++;
            NumberToClearFromStarFinalBuffer++;
        }
        NumberToClearFromStarFinalBuffer++;

        for (int i = clearIter; i <FinalBufferToFix.length ; i++) {
            FinalBufferToFix[i]=null;
        }
        bufferIterator =     NumberToClearFromStarFinalBuffer;
        ;

    }

    private static void ToStringFinaBufferToFix() {

        for (int i = 0; i < bufferIterator; i++) {
            for (int j = 0; j <FinalBufferToFix[0].length ; j++) {
                if(j%(FinalBufferToFix[0].length)==0){
                    StringToWrite +="\n";
//                    out.println(Buffer);


//                        writer2.write(Buffer);
                }
                StringToWrite +=Integer.toString(FinalBufferToFix[i][j])+" ";
                StringToWrite += ";";
            }
        }



    }

    private static void SendToFix(int[] finalBufferInt) {
        //Наполняем двумерный буффер, добавляем полученный ряд
        FinalBufferToFix[bufferIterator] = finalBufferInt;

        //Наполняем до тех пор пока темеперетупа не покажет значение !=0
        System.out.println();
        System.out.println(checkTempBMPflag + " - bufferIterator "+ bufferIterator);
        if(FinalBufferToFix[bufferIterator][9]!=0){
            //Если да, то смотрим что бы давление тоже заверщилось не ноль
            checkTempBMPflag=true;
            clearIter = bufferIterator;

        }

        //Если да, то смотрим что бы давление тоже заверщилось не ноль

        if(checkTempBMPflag){
            if(FinalBufferToFix[bufferIterator][10]!=0){

                //Заполняем пропущенные места
                FixFinalBuffer();

                checkTempBMPflag=false;
                ReadyToWriteflag = true;

                System.out.println();
                System.out.println("ReadyToWriteflag "+ReadyToWriteflag);

                System.out.println();
                System.out.println(FinalBufferToFix[bufferIterator][9] + " " +FinalBufferToFix[bufferIterator][10] + " " + bufferIterator);
                bufferIterator--;
            }
        }

        //Сделали проверки и двигаемся к следующему элементу
        bufferIterator++;


    }

    private static void FixFinalBuffer() {
        int PressCheck = 1;
        int determinator = 0;
        int NextAfterNonZero=1;
        int NumberPlusDetermin=1;

        System.out.println();
        System.out.println("Iter "+bufferIterator+ " " + FinalBufferToFix.length );

        for (int numberFinalToFix = 0; numberFinalToFix <bufferIterator ; numberFinalToFix++) {
            System.out.println("numberFinalToFix " + numberFinalToFix  );
            //Если давление !=0
            if (FinalBufferToFix[numberFinalToFix][10]!=0){
                    NextAfterNonZero = numberFinalToFix +1;
                    NumberPlusDetermin = numberFinalToFix +1;
                //От него отсчитываем сколько пустых есть строк
                while (FinalBufferToFix[NextAfterNonZero][10]==0){
                    System.out.println("NextAfterNonZero "+NextAfterNonZero);
                    if(NextAfterNonZero<(bufferIterator)){
                        NextAfterNonZero++;

                    }
                }

                //Узнаем каком у нас будет делить для строк
                determinator = (FinalBufferToFix[NextAfterNonZero][10] -FinalBufferToFix[numberFinalToFix][10])/(NextAfterNonZero-numberFinalToFix);

                //Заполняем пустые строки= первое число + делитель
                while (FinalBufferToFix[NumberPlusDetermin][10]==0){
                    FinalBufferToFix[NumberPlusDetermin][10]= FinalBufferToFix[numberFinalToFix][10]+determinator*(NextAfterNonZero-numberFinalToFix);
                }


            }
        }

    }

    private static void TakeTime() {
        final long endTime = System.currentTimeMillis();

        System.out.println("Total execution time: " + (((endTime - startTime))/1000) +" second" );
    }

    private static void ToString(int[] finalBufferInt) {
        for (int i = 0; i <finalBufferInt.length ; i++) {
            FinalBuffer[i] = Integer.toString(finalBufferInt[i]);
        }
    }

    private static void MathOper(int[] finalBuffer) {

        //Accum
        FinalBufferInt[11]  = 4400/255*FinalBufferInt[11];


        //ParaCheck;
        FinalBufferInt[15] = ((FinalBufferInt[12]&1)!=0) ? 1 : 0;

        // FuseCheck;
        FinalBufferInt[14] = ((FinalBufferInt[12]&2)!=0) ? 1 : 0;

        // ParaFired;
        FinalBufferInt[13] = ((FinalBufferInt[12]&4)!=0) ? 1 : 0;

        // FusedFired;
        FinalBufferInt[12] = ((FinalBufferInt[12]&8)!=0) ? 1 : 0;



        // LED;
        FinalBufferInt[17] = ((FinalBufferInt[16]&1)!=0) ? 1 : 0;

        // Signal;
        FinalBufferInt[16] = ((FinalBufferInt[16]&2)!=0) ? 1 : 0;

    }


    private static void FormedFinalHexBuffer(String[] StringHexVal) {
        String ByteBuffer;
        //Для наглядности выводиться данные будут в немного другом порядке чем они записываются на флешку
        //Здесь это исправить проще

        //Number
        FinalBuffer[0] = FourByteTogether(26);


        //Time
        FinalBuffer[1] = FourByteTogether(19);


        //Get AccX, AccY, AccZ,
        for (int i=1, j=2; i < 6 ; i+=2, j++) {

            //First method (add to Byte together like: 7f + ff = 7fff)
            //Second - if first number >7 add FFFF - Negative result, else 0000 - Positive;
            FinalBuffer[j] = CheckSigned(        ByteTogether(StringHexVal[i], StringHexVal[i+1]));

        }


        //GyrX,GyrY,GyrZ,
        for (int i=9, j=5; i < 14 ; i+=2, j++) {

            //First method (add to Byte together like: 7f + ff = 7fff)
            //Second - if first number >7 add FFFF - Negative result, else 0000 - Positive;
            FinalBuffer[j] = CheckSigned(        ByteTogether(StringHexVal[i], StringHexVal[i+1]));

        }

       // mpuTemp
        FinalBuffer[8] = CheckSigned(ByteTogether(StringHexVal[7], StringHexVal[8]));


        //bmpTemp, bmpPress
        for (int i=15, j=9; i < 18 ; i+=2, j++) {

            //First method (add to Byte together like: 7f + ff = 7fff)
            //Second - if first number >7 add FFFF - Negative result, else 0000 - Positive;
            FinalBuffer[j] = CheckSigned(        ByteTogether(StringHexVal[i], StringHexVal[i]));

        }


        //Accumulator
        FinalBuffer[11] = "000000"+ StringHexVal[23];

        //Parachute
        FinalBuffer[12] = StringHexVal[24];
        FinalBuffer[16] = StringHexVal[25];

        FinalBuffer[13] = "0";
        FinalBuffer[14] = "0";
        FinalBuffer[15] = "0";
        FinalBuffer[17] = "0";


    }

    private static String FourByteTogether(int i) {
        String buff = StringHexVal[i+3];
        buff += StringHexVal[i+2];
        buff += StringHexVal[i+1];
        buff += StringHexVal[i];
        return buff;
    }


    private static void WriteStartToFile() {

        String Buffer = "";

        int j = 0;
        try (BufferedWriter writer = new BufferedWriter(new OutputStreamWriter(

                //Запись в кодировке Windows-1251, чтобы csv адекватно открывался, на русском
                //Я передумал, хоть и работаем, но выводить буду на англе, пусть будет на всякий
                new FileOutputStream("d:/Test.csv"), "cp1251"))) {

            Buffer = "Num;Time, ms;Acc X;Acc Y;Acc Z;Gyr X;" +
                    "Gyr Y;Gyr Z;Temp MPU;Temp BMP;Press BMP;Batt, mV;" +
                    "ParaCheck;FuseCheck;ParaFired;FusedFired;Signal;LED;";
            writer.write(Buffer);
            //writer.flush();

        } catch (IOException e) {
            e.printStackTrace();
        }

    }

    private static void CheckSize(String [] buff) {

        for (int i = 0; i <buff.length ; i++) {
            if(buff[i].length()==1){
                buff[i]="0"+buff[i];
            }
            StringHexVal[i]=buff[i];
        }

    }

    private static String StringToDec(String byteBuffer) {
        int y = (int) Long.parseLong(byteBuffer, 16);
        String buff = Integer.toString(y);
        return buff;
    }

    private static String CheckSigned(String s) {
        String some =  s.substring(0, 1);
        if(some.equals("8")||some.equals("9")||some.equals("a")||some.equals("b")||some.equals("c")||some.equals("d")||some.equals("e")||some.equals("f")){
           if(some.length()==1){
               s="FFFFFFF"+s;
               return s;
           }
           else if(some.length()==2){
               s="FFFFFF"+s;
               return s;
           }
           else if(some.length()==3){
               s="FFFFF"+s;
               return s;
           }
           else if(some.length()==4){
               s="FFFF"+s;
               return s;
           }

        }
        else {
            if(some.length()==1){
                s="0000000"+s;
                return s;
            }
            else if(some.length()==2){
                s="000000"+s;
                return s;
            }
            else if(some.length()==3){
                s="00000"+s;
                return s;
            }
            else if(some.length()==4){
                s="0000"+s;
                return s;
            }
        }
        return null;

    }

    private static String ByteTogether(String s, String s1) {
        String buff = s;
       buff += s1;
        System.out.print(buff+" ");
        return buff;
    }

    private static void getByteArray(String s) {

        File file = new File(s);        try (FileInputStream fis = new FileInputStream(file)) {
            System.out.println("Total file size to read (in bytes) : "+ fis.available());

            int readedByte;
            byte i=0;
            while ((readedByte = fis.read()) != -1) {

                System.out.print(readedByte+" ");
                if(i==31){

                    //Отправить буффер на преобзоавние в стринг
                    buffer[i]=readedByte;
                    System.out.println();

                }
                else {
                    buffer[i]=readedByte;
                    i++;
                }
            }

        } catch (IOException e) {
            e.printStackTrace();
        }
    }


    private static String[] ToStringHex(int[] buffer) {
        for (int i = 0; i < buffer.length; i++) {
            StringHexVal[i] = Integer.toHexString(buffer[i]);

        }
        return StringHexVal;

    }


    private static void WriteToFile() {

        //Записываем в файл посторочно из 8 значений
        String Buffer = "";

        int j =0;

        /*
        try(BufferedWriter writer2 = new BufferedWriter(new OutputStreamWriter(
                //Запись в кодировке Windows-1251, чтобы csv адекватно открывался
                new FileOutputStream("d:/Test.csv"), "cp1251"))) {
*/
        try(FileWriter fw = new FileWriter(pathsFinish, true);
            BufferedWriter bw = new BufferedWriter(fw);
            PrintWriter out = new PrintWriter(bw))
        {

            try {
                Files.write(Paths.get(pathsFinish), StringToWrite.getBytes(), StandardOpenOption.APPEND);
            }catch (IOException e) {
                //exception handling left as an exercise for the reader
            }

            out.close();
            //more code

        } catch (IOException e) {
            //exception handling left as an exercise for the reader
        }

            // запись всех элементов, собрать в один Стринг

                //пройтись по каждому елементу массива



            //writer.append('\n');

            //writer2.flush();
        /*
        } catch (UnsupportedEncodingException e) {
            e.printStackTrace();
        } catch (FileNotFoundException e) {
            e.printStackTrace();
        } catch (IOException e) {
            e.printStackTrace();
        }
*/

//
//        try (FileWriter writer = new FileWriter("d:/Test.csv", false)) {
//
//        }
//        catch(IOException ex){
//
//            System.out.println(ex.getMessage());
//        }
//



    }


    private static int[] ConvertToDec(String[] hexVal) {
        int [] newHex = new int [hexVal.length];
        for (int k = 0; k <hexVal.length ; k++) {
             {
                newHex[k] = (int)Long.parseLong(hexVal[k], 16);
            }
        }

        return newHex;
    }




//
//    private static List<String> ReadFile(String s) {
//        ArrayList<String > list = new ArrayList<String>();
//        BufferedReader reader = null;
//        try {
//            reader = new BufferedReader(new FileReader(new File(s)));
//
//            String c;
//            while ((c = reader.read()) != -1) {
//                list.add(Integer.toHexString(c));
//            }
//            reader.close();
//
//        } catch (IOException e) {
//            e.printStackTrace();
//        } finally {
//            if (reader != null) {
//                try {
//                    reader.close();
//                } catch (IOException e) {
//                    e.printStackTrace();
//                }
//            }
//        }
//        return list;
//    }



    private static String[][] get2DimHexArray(byte bytesMass[]) throws IOException {
        //Получить массив байтов и разделить его на двумерный размером [Х:32]

                //создать двумерный массив Х:32
        String [][] HexVal = new String [bytesMass.length/31][32];

        int j = -1;
        int k = 0;

        for (int i = 0; i <bytesMass.length ; i++) {
            if(i%32==0&&i!=0){
                j=0;
                k++;
                System.out.println();
            }
            else {
                j++;

            }
            //System.out.print(k+" "+j+ " " + bytesMass[k][j]+" ");
            HexVal[k][j] = Integer.toHexString(bytesMass[i]);



        }

        return HexVal;
    }




    private static String[] cutByteArray(String [] array) throws IOException {

        for ( String s : array){
            if(s.length()==8){
                s = s.substring(4,7);
            }
        }



        return array;
    }


    //Обрезать лишнии обозначения в байтах(выкинуть 6 ffffff)чтобы
    // потом можно было склеить байты в один и преобразовать
    private static String [][] HexCutMetode(String [] [] list) {
        String [] [] newlist = new String[list.length][16];
        String s= "";
        String second = "";
        for (int k=0; k<list.length;k++){
            for (int j = 0; j < 31; j+=2) {

                if(list[k][j].length()==8)
                    newlist[k][j/2]=list[k][j].substring(6,8);
                else if(list[k][j].length()==1) {
                    newlist[k][j/2]="0"+list[k][j];
                }
                else {
                    newlist[k][j/2]=list[k][j];

                }


                if(list[k][j+1].length()==8){
                    newlist[k][j/2]+=list[k][j+1].substring(6,8);
                }
                else if(list[k][j+1].length()==1){
                    newlist[k][j / 2] += "0"+list[k][j + 1];
                }
                else {
                    newlist[k][j / 2] +=list[k][j + 1];
                }


                if(list[k][j].substring(0,1).equals("f")){
                    newlist[k][j / 2] = "FFFF"+newlist[k][j / 2];
                }
                else {
                    newlist[k][j / 2] = "0000"+newlist[k][j / 2];
                }



            }
        }
        return newlist;
    }



    public static ArrayList<String> addHex () {
        //Создать искустченный массив
        ArrayList<String> list = new ArrayList<>();
        list.add("8001");
        list.add("FFFE");
        list.add("8000");
        list.add("FFFF");
        list.add("0001");
        list.add("7FFF");
        list.add("0000");
        list.add("7474");
        list.add("7070");
        return list;
    }

    public static void outList (ArrayList<Integer> list){
        for (Integer x: list){
            System.out.print(x+", ");
        }
        System.out.println();
    }

    public static void outHexList (int[] list) {
        //Вывести двумерный масив Стринг



            for (int y : list) {
                System.out.print(y + " ");
            }
            System.out.println();
        }


        //Тест на переполнение памяти
//        for (int i = 100;i<list.length; i++)
//        {
//            for (int j = 0; j<list[0].length;j++)
//            {
//                System.out.print(list[i][j] + " ");
//            }
//            System.out.println();
//        }


    public static void outHexListOneDim (String[] list){
        //Вывести двумерный масив Стринг


        for (String x : list)
        {

                System.out.print(x + " ");
        }
        System.out.println();


    }
    public static void outHexList (int [][] list){
        //Вывести двумерный масив Инт

        for (int[] x : list)
        {
            for (int y : x)
            {
                System.out.print(y + " ");
            }
            System.out.println();
        }
    }






}