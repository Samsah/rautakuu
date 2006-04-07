import java.util.*;
import java.io.*;
import java.lang.*;

public class ArpojaMyydaan {
		
public static void main(String[] args){
		ProjektiValikko();
		Valitse();
	}
	
	public static void ProjektiValikko(){
		System.out.println("\nT�m� ohjelman on tehnyt Raine Liukko.\n" +
				  "rsl@rautakuu.org\n");
		System.out.println("Tervetuloa arpojen myynnin hallintaan tarkoitettuun ohjelmaan.\n" +
				 "1. Uusi myyntier�\n" +
				 "2. Jatka vanhaa myyntier��\n" +
				 "0. Lopeta");
		try{
		String projektiMenu;
		BufferedReader uusiWanha = new BufferedReader(new InputStreamReader(System.in));
		System.out.print("Valintasi > ");
		projektiMenu = uusiWanha.readLine();
		int luvuksi;
		luvuksi = Integer.parseInt(projektiMenu);
		
		switch(luvuksi)
		{
			case 0:
				System.out.println("EXIT");
				System.exit(0);
				
			case 1:
				uusiProjekti();
				Valitse();
				
				
			case 2:
				wanhaProjekti();
				Valitse();
		}
		}
		catch(IOException e)
		{
			System.out.println("Virheellinen sy�te! Yrit� uudelleen.");
			ProjektiValikko();
		}
		catch(NumberFormatException e)
		{
			System.out.println("Sy�te pit�� antaa numerona!");
			ProjektiValikko();
		}
	}
	
	public static void uusiProjekti(){
		System.out.println("P��tit luoda uuden myyntier�n. Nyt sinun tulisi t�ytt�� t�rkeit� tietoja myyntieriin liittyen.");
		
		try
		{
		String projektiNimi;
		BufferedReader projekti = new BufferedReader(new InputStreamReader(System.in));
		System.out.print("Myyntier�n nimi > ");
		projektiNimi = projekti.readLine();
		String fileName = projektiNimi + ".dat";
		FileOutputStream file = new FileOutputStream(fileName);
		ObjectOutputStream fileStream = new ObjectOutputStream(file);
		fileStream.writeObject(projektiNimi);
		fileStream.flush();
		fileStream.close();
		file.close();
		
		String arpojenMaara;
		BufferedReader maara = new BufferedReader(new InputStreamReader(System.in));
		System.out.print("\nMyyt�vien arpojen m��r� > ");
		arpojenMaara = maara.readLine();
		int oikeellisuus;
		oikeellisuus = Integer.parseInt(arpojenMaara);
		String fileName2 = projektiNimi + "Maara.dat";
		FileOutputStream file2 = new FileOutputStream(fileName2);
		ObjectOutputStream fileStream2 = new ObjectOutputStream(file2);
		fileStream2.writeObject(arpojenMaara);
		fileStream2.flush();
		fileStream.close();
		file2.close();
		
		String arpojenHinta;
		BufferedReader hinta = new BufferedReader(new InputStreamReader(System.in));
		System.out.print("\nYhden arvan hinta > ");
		arpojenHinta = hinta.readLine();
		int oikeellisuus2;
		oikeellisuus2 = Integer.parseInt(arpojenHinta);
		String fileName3 = projektiNimi + "Hinta.dat";
		FileOutputStream file3 = new FileOutputStream(fileName3);
		ObjectOutputStream fileStream3 = new ObjectOutputStream(file3);
		fileStream3.writeObject(arpojenHinta);
		fileStream3.flush();
		fileStream3.close();
		file3.close();
		
		}
		catch(IOException e)
		{
			System.out.println("Sy�tteen antaminen ei onnistunut! Yrit� uudelleen.");
			ProjektiValikko();
		}
		catch(NumberFormatException e)
		{
			System.out.println("Virheellinen sy�te! Sy�te ei mahdollisesti saa sis�lt�� merkkej�, jotka eiv�t ole numeroita.");
			ProjektiValikko();
		}
		
		
	}
	
	public static void wanhaProjekti(){
		try
		{
		String wanhaProjekti;
		BufferedReader filename = new BufferedReader(new InputStreamReader(System.in));
		System.out.print("\nSy�t� vanhan myyntier�n nimi > ");
		wanhaProjekti = filename.readLine();
		String wanhaProjektiMaara = wanhaProjekti + "Maara";
		String wanhaProjektiHinta = wanhaProjekti + "Hinta";
		String fileName = wanhaProjekti + ".dat";
		String fileName2 = wanhaProjektiMaara + ".dat";
		String fileName3 = wanhaProjektiHinta + ".dat";
		
		FileInputStream wanha = new FileInputStream(new File(fileName));
		BufferedReader filebuffer = new BufferedReader(new InputStreamReader(wanha));
		String[] rivi = new String[3];
		rivi[0] = filebuffer.readLine();
		filebuffer.close();
		wanha.close();
		
		FileInputStream wanha2 = new FileInputStream(new File(fileName2));
		BufferedReader filebuffer2 = new BufferedReader(new InputStreamReader(wanha2));
		rivi[1] = filebuffer2.readLine();
		filebuffer2.close();
		wanha2.close();
		
		FileInputStream wanha3 = new FileInputStream(new File(fileName3));
		BufferedReader filebuffer3 = new BufferedReader(new InputStreamReader(wanha3));
		rivi[2] = filebuffer3.readLine();
		filebuffer3.close();
		wanha3.close();
		String[] nro = new String[3];
		nro[0] = "Myyntier�n nimi: ";
		nro[1] = "Arpoja j�ljell�: ";
		nro[2] = "Yksi arpa maksaa: ";
		
		for(int i = 0; i < 3; i++)
		{
			System.out.println(nro[i] + rivi[i]);
		}
		}
		catch(FileNotFoundException e)
		{
			System.out.println("Myyntier�� ei l�ydy. Sy�t� myyntier�n nimi uudelleen.");
			wanhaProjekti();
		}
		catch(IOException e)
		{
			System.out.println("Virheellinen sy�te! Yrit� uudelleen.");
			wanhaProjekti();
		}
	}
	
	public static void Valitse(){
            try
            {
	    System.out.println("1. Myy arpoja\n" +
		"2. Arpojen m��ritykset\n" +
		"0. Lopetus");
	    String valinta;
	    BufferedReader syote = new BufferedReader(new InputStreamReader(System.in));
	    System.out.print("Sy�t� valinta > ");
	    valinta = syote.readLine();
	    int valinta2 = Integer.parseInt(valinta);
	    switch(valinta2)
	    {
	    case 1:
	    	MyyArpoja();
	    	break;
	    	
	    case 2:
	    	ArpojenMaaritykset();
	    	break;
	    	
	    case 0:
	    	System.exit(0);
	    	
	    	default:
	    	System.out.println("Valitse joku luettelon vaihtoehdoista!");
	    	Valitse();
	    }
	    
	    }
	    catch(IOException e)
	    {
	    	System.out.println("Virheellinen sy�te! Yrit� uudelleen.");
                Valitse();
	    }
	    catch(NumberFormatException e)
	    {
	    	System.out.println("Sy�tteen pit�� olla numero! Sy�t� valintasi uudelleen.");
                Valitse();
	    }
		
	}
	
	public static void MyyArpoja(){
		System.out.println("T��lt� voit suorittaa arpojen myymisen. Myynneist� pidet��n tilastoa johon merkit��n ostaja sek� kuinka paljon t�m� k�ytti rahaa arpoihin.");
	}
	
	public static void ArpojenMaaritykset(){
		System.out.println("T��lt� voit m��ritell� arpojen hinnat ja paljonko niit� myyd��n.\n");
		System.out.println("1. M��rit� arpojen m��r�" +
				"\n2. M��rit� arpojen hinta" +
				"\n0. Takaisin p��valikkoon\n");
                try
                {
                	String valinta;
                	BufferedReader syote = new BufferedReader(new InputStreamReader(System.in));
                	System.out.print("Sy�t� valinta > ");
                	valinta = syote.readLine();
                	int valinta2 = Integer.parseInt(valinta);
                
                	switch(valinta2)
                	{
                    	case 1:
				
                        	break;
                        
                    	case 2:
			    
                            	break;
                }
                }
                catch(IOException e)
                {
                    System.out.println("Virheellinen sy�te! Yrit� uudelleen.");
                    ArpojenMaaritykset();
                }
                catch(NumberFormatException e)
	    {
	    	System.out.println("Sy�tteen pit�� olla numero! Sy�t� valintasi uudelleen.");
                ArpojenMaaritykset();
	    }
	}
	

}
