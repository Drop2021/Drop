#!/usr/bin/env python
# coding: utf-8

# In[1]:


from datetime import datetime, timedelta,date
import pandas as pd
get_ipython().run_line_magic('matplotlib', 'inline')
import matplotlib.pyplot as plt
import numpy as np
import keras
import tensorflow as tf
from keras.layers import Dense
from keras.models import Sequential
from keras.callbacks import EarlyStopping
from keras.utils import np_utils
from keras.layers import LSTM
from sklearn.model_selection import KFold, cross_val_score, train_test_split
from keras.layers import Dropout


# In[2]:


df=pd.read_csv("Yasser200A.csv")


# In[3]:


df.head(5)


# In[4]:


df.info()


# In[5]:


# df['IS_INVALID_READ'].unique()


# In[6]:


df=df.loc[df['DEVICE_ID'].isin(['NWC-554101'])]
df['DEVICE_CONSUMPTION_DATETIME'] = pd.to_datetime(df['DEVICE_CONSUMPTION_DATETIME'], dayfirst=True).dt.strftime('%Y-%m')


# In[7]:


#groupby date and sum the sales
dff = df.groupby('DEVICE_CONSUMPTION_DATETIME').CONSUMPTION_VALUE.sum().reset_index()


# In[8]:


#create new dataframe from transformation from time series to supervised
df_supervised = dff.copy()
#adding lags
for inc in range(1,15):
    field_name = 'lag_' + str(inc)
    df_supervised[field_name] = df_supervised['CONSUMPTION_VALUE'].shift(inc)


# In[9]:


#drop null values
df_supervised = df_supervised.dropna().reset_index(drop=True)


# In[10]:


# import MinMaxScaler 
# and create a new dataframe for LSTM model
from sklearn.preprocessing import MinMaxScaler
df_model = df_supervised.drop(['CONSUMPTION_VALUE','DEVICE_CONSUMPTION_DATETIME'],axis=1)
# df_model=df_supervised.copy()
#split train and test set
train_set, test_set = df_model[0:-6].values, df_model[-6:].values


# #apply Min Max Scaler
scaler = MinMaxScaler(feature_range=(-1, 1))
scaler = scaler.fit(train_set)
# reshape training set
train_set = train_set.reshape(train_set.shape[0], train_set.shape[1])
train_set_scaled = scaler.transform(train_set)

# reshape test set
test_set = test_set.reshape(test_set.shape[0], test_set.shape[1])
test_set_scaled = scaler.transform(test_set)


# In[11]:


X_train, y_train = train_set_scaled[:, 1:], train_set_scaled[:, 0:1]
X_train = X_train.reshape(X_train.shape[0], 1, X_train.shape[1])
X_test, y_test = test_set_scaled[:, 1:], test_set_scaled[:, 0:1]
X_test = X_test.reshape(X_test.shape[0], 1, X_test.shape[1])
model = Sequential()
model.add(LSTM(80, batch_input_shape=(1, X_train.shape[1], X_train.shape[2]),activation='relu'))
model.add(Dropout(0.2))
model.add(Dense(1,activation='relu'))
model.compile(loss='mse', optimizer='adam',metrics=['mean_absolute_error','mse'])
history=model.fit(X_train, y_train, epochs=200, batch_size=20, verbose=1, shuffle=False)
df_supervised.info()


# In[12]:


y_pred = model.predict(X_test,batch_size=1)


# In[13]:


#reshape y_pred
y_pred = y_pred.reshape(y_pred.shape[0], 1, y_pred.shape[1])


# In[14]:


#rebuild test set for inverse transform
pred_test_set = []
for index in range(0,len(y_pred)):
    print (np.concatenate([y_pred[index],X_test[index]],axis=1))
    pred_test_set.append(np.concatenate([y_pred[index],X_test[index]],axis=1))


# In[15]:


print(model.summary())
#test the model
score = model.evaluate(X_test, y_test, verbose=0)
print(model.metrics_names)
print(score)
# print('Test loss:', score[0])
# print('Test mean absolute error:', score[1])
# print('Test mean sequared error:', score[2])


# In[16]:


pred_test_set[0]

#reshape pred_test_set
pred_test_set = np.array(pred_test_set)
pred_test_set = pred_test_set.reshape(pred_test_set.shape[0], pred_test_set.shape[2])
#inverse transform
pred_test_set_inverted = scaler.inverse_transform(pred_test_set)
#create dataframe that shows the predicted sales
result_list = []
sales_dates = list(dff[-7:]['DEVICE_CONSUMPTION_DATETIME'])
act_sales = list(dff[-7:].CONSUMPTION_VALUE)
for index in range(0,len(pred_test_set_inverted)):
    result_dict = {}
    result_dict['pred_value'] = int(pred_test_set_inverted[index][0] + act_sales[index])
    result_dict['DEVICE_CONSUMPTION_DATETIME'] = sales_dates[index+1]
    result_list.append(result_dict)
df_result = pd.DataFrame(result_list)
df_result


# In[17]:


#merge with actual sales dataframe
df_pred = pd.merge(dff,df_result,on='DEVICE_CONSUMPTION_DATETIME',how='left')
df_pred


# In[18]:


dd=np.array([[[10588500,2234000,43539000,4034500,5005600,899006,700568,808654,97654,187650,1455661,1856762,133456]]])
zz=model.predict(dd)
zz

#105500,2234000,4353000,4034500,5005600,89006,7008,808654,97654,187650,14561,18762,133456


# In[19]:


# Convert the model.
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the model.
with open('model.tflite', 'wb') as f:
      f.write(tflite_model)


# In[20]:


# import tensorflow

# #Save the model to saved_model.pbtxt

# tensorflow.keras.models.save_model(model,'saved_model.pbtxt')

# converter = tensorflow.lite.TFLiteConverter.from_keras_model(model=model)
# # Create a tflite model object from TFLite Converter

# tfmodel = converter.convert()

# # Save TFLite model into a .tflite file 

# open("consumption.tflite","wb").write(tfmodel)


# In[ ]:





# In[ ]:





# In[ ]:




