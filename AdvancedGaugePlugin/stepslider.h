#ifndef STEPSLIDER_H
#define STEPSLIDER_H

#include <QSlider>
#include <QTimer>

class StepSlider : public QSlider
{
    Q_OBJECT
public:
    explicit StepSlider(QWidget *parent = nullptr);
    void setRepeatDelay(int ms);
    void setRepeatInterval(int ms);

signals:
    void stepApplied(double delta);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private slots:
    void onRepeatTimeout();

private:
    QTimer *m_repeatTimer;
    double m_lastDelta;
    int m_repeatDelay;
    int m_repeatInterval;
    void startRepeat(double delta);
    void stopRepeat();
    int stepFromPos(const QPoint &pos) const;
    double deltaFromStep(int step) const;
};

#endif
