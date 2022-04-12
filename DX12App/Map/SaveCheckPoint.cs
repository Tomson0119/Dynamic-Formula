using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using System.IO;

public class SaveCheckPoint : MonoBehaviour
{
    // Start is called before the first frame update
    void Start()
    {
        Debug.Log("Start Writing\r\n");
        FileStream file = new FileStream(Application.dataPath + "/CheckPoint.tmap", FileMode.Create);
        StreamWriter writer = new StreamWriter(file);
        for (int i = 0; i < transform.childCount; ++i)
        {
            GameObject obj = transform.GetChild(i).gameObject;
            
            //ÁÂÇ¥
            Vector3 pos = obj.transform.position;

            //ÄõÅÍ´Ï¾ð È¸Àü°¢
            var orientation = obj.transform.rotation;

            Vector3 Extent = obj.transform.localScale / 2;

            writer.Write(Extent.x.ToString() + " " + Extent.y.ToString() + " " + Extent.z.ToString() + " " + pos.x.ToString() + " " + pos.y.ToString() + " " + pos.z.ToString() +
                " " + orientation.x.ToString() + " " + orientation.y.ToString() + " " + orientation.z.ToString() + " " + orientation.w.ToString() + "\r\n");

            Debug.Log("Successfully Write CheckPoint" + name + "\r\n");
            Debug.Log(name + " Transform :" + "\r\n" + obj.transform.localToWorldMatrix.ToString() + "\r\n");
        }
        writer.Flush();
        writer.Close();
    }

    // Update is called once per frame
    void Update()
    {
        
    }
}
